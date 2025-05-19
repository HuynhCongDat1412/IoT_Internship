#include <Arduino.h>
#include <Wire.h>

#include "SSD1306Wire.h"

// Khởi tạo màn hình SH1106 (I2C)
SSD1306Wire display(0x3C, 7, 15); // Địa chỉ I2C, SDA = 7, SCL = 15

void setup() {
  display.init();
  display.clear();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16); // Chọn font chữ
  display.drawString(0, 0, "Hello Oled!!"); // Hiển thị văn bản
  display.display(); // Cập nhật nội dung lên màn hình
}

void loop() {
  // Không làm gì trong loop
}