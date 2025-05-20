#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include <RadioLib.h>

Module mod(39, 42, 40, 41, SPI);
SX1280 radio(&mod);
#define carrier_frequency 2450.0
#define bandwidth 1625.0
#define spreading_factor 7
#define coding_rate 5
#define sync_word 0x12
#define output_power 2
#define preamble_length 20
// Khởi tạo cho các tham số RF



// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif

void setFlag(void) {
  // we got a packet, set the flag
  receivedFlag = true;
}


// Khởi tạo màn hình OLED (I2C), SDA = 7, SCL = 15
SSD1306Wire display(0x3C, 7, 15);

String lastMsg = ""; // Lưu tin nhắn trước đó

void setup() {
  Serial.begin(115200);
  delay(2000);

  //------------------------Khoi tao RF
  SPI.begin(36, 37, 35);
  int state = radio.begin(
    carrier_frequency, 
    bandwidth, 
    spreading_factor, 
    coding_rate, 
    sync_word, 
    output_power, 
    preamble_length);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }
  // set the function that will be called
  // when new packet is received
  radio.setPacketReceivedAction(setFlag);
    // start listening for LoRa packets
  Serial.print(F("[SX1280] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }

  // you can also change the settings at runtime
  // and check if the configuration was changed successfully

  // set carrier frequency to 2410.5 MHz
  if (radio.setFrequency(carrier_frequency) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    Serial.println(F("Selected frequency is invalid for this module!"));
    while (true) { delay(10); }
  }

  // set bandwidth to 203.125 kHz
  if (radio.setBandwidth(bandwidth) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
    Serial.println(F("Selected bandwidth is invalid for this module!"));
    while (true) { delay(10); }
  }

  // set spreading factor to 10
  if (radio.setSpreadingFactor(spreading_factor) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
    Serial.println(F("Selected spreading factor is invalid for this module!"));
    while (true) { delay(10); }
  }

  // set coding rate to 6
  if (radio.setCodingRate(coding_rate) == RADIOLIB_ERR_INVALID_CODING_RATE) {
    Serial.println(F("Selected coding rate is invalid for this module!"));
    while (true) { delay(10); }
  }

  // set output power to -2 dBm
  if (radio.setOutputPower(output_power) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
    Serial.println(F("Selected output power is invalid for this module!"));
    while (true) { delay(10); }
  }

  // set LoRa preamble length to 16 symbols (accepted range is 2 - 65535)
  if (radio.setPreambleLength(preamble_length) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
    Serial.println(F("Selected preamble length is invalid for this module!"));
    while (true) { delay(10); }
  }

  // disable CRC
  if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
    Serial.println(F("Selected CRC is invalid for this module!"));
    while (true) { delay(10); }
  }

  Serial.println(F("All settings succesfully changed!"));

  //-----------------------------------------------------------khoi tao led
  display.init();
  display.clear();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
}

void loop() {
  if(receivedFlag) {
    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String
    String str;
    int state = radio.readData(str);


  if (state == RADIOLIB_ERR_NONE) {
    Serial.print("[RX] Data: ");
    Serial.println(str);

    display.clear();
    // Hiển thị thông số RF ở dòng đầu tiên
    String rfInfo = "F:" + String(carrier_frequency, 1) + "MHz ";
    //rfInfo += "BW:" + String(bandwidth, 1) + "kHz ";
    //rfInfo += "SF:" + String(spreading_factor);
    //rfInfo += " CR:" + String(coding_rate);
    rfInfo += " SW:0x" + String(sync_word, HEX);
    //rfInfo += " P:" + String(output_power) + "dBm";
    display.drawString(0, 0, rfInfo);

    if (state == RADIOLIB_ERR_NONE) {
      // packet was successfully received
      Serial.println(F("[SX1280] Received packet!"));

      // print data of the packet
      Serial.print(F("[SX1280] Data:\t\t"));
      Serial.println(str);
      display.drawString(0, 12, "Data: " + str);


      // print RSSI (Received Signal Strength Indicator)
      Serial.print(F("[SX1280] RSSI:\t\t"));
      Serial.print(radio.getRSSI());
      Serial.println(F(" dBm"));
      display.drawString(0, 24, "RSSI: " + String(radio.getRSSI(), 1) + " dBm");


      // print SNR (Signal-to-Noise Ratio)
      Serial.print(F("[SX1280] SNR:\t\t"));
      Serial.print(radio.getSNR());
      Serial.println(F(" dB"));
      display.drawString(0, 36, "SNR: " + String(radio.getSNR(), 1) + " dB");


      // print the Frequency Error
      // of the last received packet
      Serial.print(F("[SX1280] Frequency Error:\t"));
      Serial.print(radio.getFrequencyError());
      Serial.println(F(" Hz"));
      display.drawString(0, 48, "FreqErr: " + String(radio.getFrequencyError(), 1) + " Hz");

      // Gửi ACK lại cho master
      int ackState = radio.transmit("ACK");
      if (ackState == RADIOLIB_ERR_NONE) {
        Serial.println("[RX] Sent ACK!");
      } else {
        Serial.print("[RX] Failed to send ACK, code ");
        Serial.println(ackState);
      }

    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial.println(F("CRC error!"));

    } else {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);

    }

    // put module back to listen mode
    radio.startReceive();
  }

    // // Nếu là số (counter), hiển thị ở dòng 2, còn lại là tin nhắn ở dòng 1
    // bool isNumber = true;
    // for (size_t i = 0; i < str.length(); i++) {
    //   if (!isDigit(str[i])) {
    //     isNumber = false;
    //     break;
    //   }
    // }
    // if (isNumber) {
    //   // Hiển thị tin nhắn trước đó ở dòng 1, counter ở dòng 2
    //   display.drawString(0, 20, lastMsg);
    //   display.drawString(0, 40, str);
    // } else {
    //   // Lưu lại tin nhắn, hiển thị ở dòng 1
    //   lastMsg = str;
    //   display.drawString(0, 20, str);
    //   // Gửi ACK lại cho transmitter
    //   int ackState = radio.transmit("ACK");
    //   if (ackState == RADIOLIB_ERR_NONE) {
    //     Serial.println("[RX] Sent ACK!");
    //   } else {
    //     Serial.print("[RX] Failed to send ACK, code ");
    //     Serial.println(ackState);
    //   }
    // }
    display.display();
  }
}