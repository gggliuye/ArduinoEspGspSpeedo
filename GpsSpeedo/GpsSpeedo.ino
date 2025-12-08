
// #include <WiFi.h>
#include "gps_gtu8.h"
#include "display_ssd1306.h"


#if CONFIG_IDF_TARGET_ESP32C3
// ESP32-C3 specific
// https://github.com/sidharthmohannair/Tutorial-ESP32-C3-Super-Mini/blob/main/docs/examples/Blink/README.md
#define LED_PIN 8
// turn the LED on (LOW because the LED is inverted)
#define LED_LIGHT_OFF HIGH
#define LED_LIGHT_ON LOW
#else
// Regular ESP32 (e.g. DevKit V1)
#define LED_PIN 2
#define LED_LIGHT_OFF LOW
#define LED_LIGHT_ON HIGH
#endif   // #if CONFIG_IDF_TARGET_ESP32C3

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  // esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // wakeup on LOW

  setCpuFrequencyMhz(160);
  DEBUG_LOG("CPU Freq set to: %d MHz\n", getCpuFrequencyMhz());

  GpsInit();
  SetUpDisplay();

  DEBUG_LOG("Listening to UART sensor...\n");
}

void loop() {
  if (GpsGetLastest()) {
    pinMode(LED_PIN, LED_LIGHT_ON);
  } else {
    pinMode(LED_PIN, LED_LIGHT_OFF);
  }

  DisplayLoop();

  // since the gps has freq=1, more update is futile
  delay(1000);
}
