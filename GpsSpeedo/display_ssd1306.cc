

#include <Wire.h>
#include "display_ssd1306.h"
#include "gps_gtu8.h"

// on a ESP32-C3-MINI-1 I used SCK pin 3 and SDA pin 2, e.g.
// https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
// Declaration for an SSD1306 display_ connected to I2C (SDA, SCL pins)
#define SCREEN_SCK 3
#define SCREEN_SDA 2
#define SCREEN_WIDTH 128 // OLED display_ width, in pixels
#define SCREEN_HEIGHT 64 // OLED display_ height, in pixels
#define TEXT_SIZE_TIME 2
#define TEXT_SIZE_GPS 1
#define TEXT_SIZE_VEL 4
#define LINE_HEIGHT 10

class MyDisplay : public Adafruit_SSD1306 {
public:
  MyDisplay(uint8_t w, uint8_t h, TwoWire *twi, int8_t rst_pin)
    : Adafruit_SSD1306(w, h, twi, rst_pin) {}

  int16_t getCursorX() { return cursor_x; }
  int16_t getCursorY() { return cursor_y; }
};

MyDisplay display_(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void SetUpDisplay() {
  Wire.begin(SCREEN_SDA, SCREEN_SCK);

  // Setup the display_
  if (!display_.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    DEBUG_LOG("SSD1306 allocation failed! Try again...\n");
    for(;;);
  }
  delay(1000);
  display_.clearDisplay();

  display_.ssd1306_command(SSD1306_SETCONTRAST);
  display_.ssd1306_command(0x5F); // 0x00~0xFF，the low, the less power consumed

  display_.setTextColor(SSD1306_WHITE);
  display_.display();
}

void DisplayLoop() {
  // display time
  int current_h = 0;
  int block_h = LINE_HEIGHT * TEXT_SIZE_TIME;
  int hour, minute;
  SystemStateGetTime(&hour, &minute, nullptr);
  display_.fillRect(0, current_h, SCREEN_WIDTH, block_h, SSD1306_BLACK);  // clear line
  display_.setTextSize(TEXT_SIZE_TIME);
  display_.setCursor(0, current_h);
  display_.printf("%02d:%02d", hour, minute);

  current_h += block_h;
  block_h = LINE_HEIGHT * TEXT_SIZE_GPS;
  display_.fillRect(0, current_h, SCREEN_WIDTH, block_h, SSD1306_BLACK);
  display_.setCursor(0, current_h);
  display_.setTextSize(TEXT_SIZE_GPS);
  display_.print("lat:");
  display_.print(gps_data.lat, 1);
  display_.print(", lng:");
  display_.print(gps_data.lng, 1);

  current_h += block_h;
  block_h = LINE_HEIGHT * TEXT_SIZE_VEL;
  display_.fillRect(0, current_h, SCREEN_WIDTH, block_h, SSD1306_BLACK);
  display_.setTextSize(TEXT_SIZE_VEL);
  display_.setCursor(0, current_h);
  display_.print(gps_data.speed_kmph, 1);
  display_.setTextSize(TEXT_SIZE_TIME);
  display_.setCursor(display_.getCursorX(), display_.getCursorY() + (TEXT_SIZE_VEL - TEXT_SIZE_TIME) * LINE_HEIGHT);
  display_.print("kmh");

  display_.display();
}
