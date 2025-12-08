
#include "gps_gtu8.h"
#include "driver/uart.h"
// https://github.com/mikalhart/TinyGPSPlus
#include <TinyGPSPlus.h>

#define UART_PORT_NUM UART_NUM_1
#define UART_BAUD_RATE 9600
#define UART_BUF_SIZE 256

GpsData gps_data;
TinyGPSPlus gps;
static bool new_gps_ = false;
static bool debug_ = false;
static char data_[UART_BUF_SIZE];

bool GpsGetLastest() {
  if (new_gps_) {
    new_gps_ = false;
    return true;
  }
  return false;
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
      gps_data.real_time = gps.time.value();
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
