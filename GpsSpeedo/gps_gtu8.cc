
#include "gps_gtu8.h"
#include "driver/uart.h"
// https://github.com/mikalhart/TinyGPSPlus
#include <TinyGPSPlus.h>

#define UART_PORT_NUM UART_NUM_1
#define UART_BAUD_RATE 9600
#define UART_BUF_SIZE 256
#define TIME_ZONE "CST-8"    // we could further compute timezone with GPS position

GpsData gps_data;
TinyGPSPlus gps;
static bool new_gps_ = false;
static bool debug_ = false;
static char data_[UART_BUF_SIZE];
static int64_t offset_time_sec_ = 0;

bool GpsGetLastest() {
  if (new_gps_) {
    new_gps_ = false;
    return true;
  }
  return false;
}

static time_t ymd_to_unix(int year, int month, int day, int hour, int min, int sec) {
  unsetenv("TZ");  // reset TZ to avoid taking into account the offset
  struct tm t;
  t.tm_year = year - 1900;  // tm_year is years since 1900
  t.tm_mon = month - 1;     // tm_mon is 0-11
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min = min;
  t.tm_sec = sec;
  t.tm_isdst = -1;  // let mktime determine DST
  time_t result = mktime(&t);
  setenv("TZ", TIME_ZONE, 1);  // set timezone environment variable
  tzset();                        // apply it
  return result;
}

static void UpdateGpsTime() {
  int64_t current_ms = esp_timer_get_time();
  static int64_t last_upated_time = 0;
  if (last_upated_time > 0 && current_ms - last_upated_time < 10000000) {
    return;
  }
  last_upated_time = current_ms;

  DEBUG_LOG("[GPS] UNIX time: %d-%d-%d %d:%d:%d\n", gps.date.year(), gps.date.month(), gps.date.day(),
      gps.time.hour(), gps.time.minute(), gps.time.second());
  // update realtime data
  time_t unix_time = ymd_to_unix(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(),
                                 gps.time.minute(), gps.time.second());

  struct tm local_tm;
  localtime_r(&unix_time, &local_tm);

  char timestr_buffer[64];
  memset(timestr_buffer, 0, sizeof(timestr_buffer));
  strftime(timestr_buffer, sizeof(timestr_buffer), "%Y-%m-%d %H:%M:%S %Z", &local_tm);

  DEBUG_LOG("[GPS] %s Local time: %s\n", getenv("TZ"), timestr_buffer);

  gps_data.real_time = local_tm.tm_hour * 3600 + local_tm.tm_min * 60 + local_tm.tm_sec;
  // SystemStateSetRealTime(gps_data.real_time, false);

  int64_t offset_time_sec = gps_data.real_time - current_ms / 1000000;
  // update the value if the cahnge is too large
  int64_t diff = offset_time_sec - offset_time_sec_;
  if (diff < 30 && diff > -30) {
    return;
  }
  offset_time_sec_ = offset_time_sec;
  DEBUG_LOG("[SYS] Set real time : %ld %lld\n", gps_data.real_time, offset_time_sec_);
}

int64_t SystemStateGetGlobalRealTime() { return offset_time_sec_ + esp_timer_get_time() / 1000000; }

void SystemStateGetTime(int* hour, int* minute, int* second) {
  int64_t time_sec = esp_timer_get_time() / 1000000 + offset_time_sec_;
  float time_today = time_sec % 86400;

  *hour = time_today / 3600.0f;
  if (minute != NULL) {
    float time_left = time_today - 3600.0f * floorf(*hour);
    *minute = time_left / 60.0f;
    if (second != NULL) {
      *second = (time_left - 60.0f * floorf(*minute));
    }
  }
}

void GpsUartReadingLoop(void* arg) {
  while (1) {
    size_t length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_PORT_NUM, (size_t*)&length));
    length = uart_read_bytes(UART_PORT_NUM, data_, length, 100);
    if (length > 0) {
      for (size_t i = 0; i < length; i++) {
        gps.encode(data_[i]);
      }
      if (debug_) {
        DEBUG_LOG("%s", data_);
      }
    }

    if (gps.time.isUpdated()) {
      UpdateGpsTime();
    }

    if (gps.location.isUpdated()) {
      gps_data.lat = gps.location.lat();
      gps_data.lng = gps.location.lng();
      gps_data.alt = gps.altitude.meters();
      gps_data.speed_kmph = gps.speed.kmph();
      gps_data.age = gps.location.age();
      // gps_data.time = gps.time.value() + gps.time.age() - gps.location.age();
      gps_data.satellites = gps.satellites.value();

      new_gps_ = true;
      vTaskDelay(pdMS_TO_TICKS(300));
    } else {
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
  vTaskDelete(NULL);
}

// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html#introduction
void GpsInit() {
  setenv("TZ", TIME_ZONE, 1);       // set timezone environment variable
  tzset();                        // apply it

  const uart_config_t uart_config = {.baud_rate = UART_BAUD_RATE,
                                     .data_bits = UART_DATA_8_BITS,
                                     .parity = UART_PARITY_DISABLE,
                                     .stop_bits = UART_STOP_BITS_1,
                                     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
  uart_param_config(UART_PORT_NUM, &uart_config);
  uart_set_pin(UART_PORT_NUM, GPS_PIN_TXD, GPS_PIN_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0));

  DEBUG_LOG("[GPS] Serial 2 started at %d baud rate.\n", UART_BAUD_RATE);
  xTaskCreatePinnedToCore(GpsUartReadingLoop, "GPS Read", 2048, NULL, 3, NULL, 0);
}
