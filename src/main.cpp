#include <Arduino.h>
#include <RadioLib.h>

Module mod(39, 42, 40, 41, SPI);
SX1280 radio(&mod);
uint32_t counter = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Initializing SX1280...");
  SPI.begin(36, 37, 35);
  int state = radio.begin();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Init failed, code ");
    Serial.println(state);
    while (true);
  }
}

void loop() {
  // 1. Gửi chuỗi 1
  String msg1 = "Hello anh Phat!";
  Serial.print("[TX] Sending: ");
  Serial.println(msg1);
  int state = radio.transmit(msg1);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Sent msg1, waiting for ACK...");
    // 2. Đợi nhận xác nhận từ RX
    String ack;
    state = radio.receive(ack, 6000); // timeout 2s
    if (state == RADIOLIB_ERR_NONE && ack == "ACK") {
      Serial.println("Received ACK!");
      // 3. Gửi biến đếm
      String msg2 = String(counter);
      Serial.print("[TX] Sending counter: ");
      Serial.println(msg2);
      state = radio.transmit(msg2);
      if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Counter sent!");
        counter++;
      } else {
        Serial.println("Failed to send counter!");
      }
    } else {
      Serial.println("No ACK received!");
    }
  } else {
    Serial.println("Failed to send msg1!");
  }
  delay(2000);
}