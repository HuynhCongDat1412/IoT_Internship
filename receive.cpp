#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include <RadioLib.h>

Module mod(39, 42, 40, 41, SPI);
SX1280 radio(&mod);

// Khởi tạo màn hình OLED (I2C), SDA = 7, SCL = 15
SSD1306Wire display(0x3C, 7, 15);

String lastMsg = ""; // Lưu tin nhắn trước đó

void setup() {
  Serial.begin(115200);
  delay(2000);
  SPI.begin(36, 37, 35);
  int state = radio.begin();
  display.init();
  display.clear();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

void loop() {
  String str;
  int state = radio.receive(str);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.print("[RX] Data: ");
    Serial.println(str);

    display.clear();
    // Nếu là số (counter), hiển thị ở dòng 2, còn lại là tin nhắn ở dòng 1
    bool isNumber = true;
    for (size_t i = 0; i < str.length(); i++) {
      if (!isDigit(str[i])) {
        isNumber = false;
        break;
      }
    }
    if (isNumber) {
      // Hiển thị tin nhắn trước đó ở dòng 1, counter ở dòng 2
      display.drawString(0, 0, lastMsg);
      display.drawString(0, 20, str);
    } else {
      // Lưu lại tin nhắn, hiển thị ở dòng 1
      lastMsg = str;
      display.drawString(0, 0, str);
      // Gửi ACK lại cho transmitter
      int ackState = radio.transmit("ACK");
      if (ackState == RADIOLIB_ERR_NONE) {
        Serial.println("[RX] Sent ACK!");
      } else {
        Serial.print("[RX] Failed to send ACK, code ");
        Serial.println(ackState);
      }
    }
    display.display();
  }
}