

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
#define TEXT_SIZE 2
#define LINE_HEIGHT (10 * 2)

Adafruit_SSD1306 display_(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

static void CleanLine(size_t line_id) {
  display_.fillRect(0, line_id * LINE_HEIGHT, SCREEN_WIDTH, LINE_HEIGHT, SSD1306_BLACK);  // clear line
}


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

  display_.setTextSize(TEXT_SIZE);
  display_.setTextColor(SSD1306_WHITE);
  display_.display();
}

static int cnt_ = 0;
void DisplayLoop() {
  CleanLine(0);  // clear line
  display_.setCursor(0, 0);
  display_.print("CNT ");
  display_.print(cnt_++);

  CleanLine(1);  // clear line
  CleanLine(2);  // clear line
  display_.setCursor(0, LINE_HEIGHT);
  display_.print(gps_data.lat, 1);
  display_.print(" ");
  display_.print(gps_data.lng, 1);
  display_.print(" ");
  display_.print(gps_data.speed_kmph, 1);

  display_.display();
}
