#include <Arduino.h>
#include <RadioLib.h>
#include <Adafruit_NeoPixel.h>

Module mod(39, 42, 40, 41, SPI);
SX1280 radio(&mod);
uint32_t count = 0; // Biến đếm gửi tin nhắn

#define LED_PIN 48
#define NUM_LEDS 1
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Khởi tạo cho các tham số RF
#define carrier_frequency 2450.0
#define bandwidth 1625.0
#define spreading_factor 7
#define coding_rate 5
#define sync_word 0x12
#define output_power 2
#define preamble_length 20

enum State { IDLE, WAIT_TRANSMIT_DONE,SEND_MAIN, WAIT_BEFORE_LISTEN, SEND_LISTEN, WAIT_ACK, ACK_RECEIVED  };
State masterState = IDLE;
uint32_t lastAction = 0;
bool ackReceived = false; // Biến kiểm tra đã nhận ACK chưa

// save transmission state between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// this function is called when a complete packet
// is transmitted by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // we sent a packet, set the flag
  transmittedFlag = true;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  strip.begin();
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.show();
  Serial.println("Initializing SX1280...");
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
  // when packet transmission is finished
  radio.setPacketSentAction(setFlag);

  // start transmitting the first packet
  Serial.print(F("[SX1280] Sending first packet ... "));

  // you can transmit C-string or Arduino string up to
  // 256 characters long
  transmissionState = radio.startTransmit("Hello World!");

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
}

void loop() {
  static String mainMsg = "Hello anh Phat!";
  static int retryCount = 0;

  switch (masterState) {
    case IDLE:
      masterState = SEND_MAIN;
      break;

    case SEND_MAIN:
      Serial.println("[TX] Sending main message...");
      transmissionState = radio.startTransmit(mainMsg);
      strip.setPixelColor(0, strip.Color(255, 0, 0)); // LED đỏ
      strip.show();
      masterState = WAIT_TRANSMIT_DONE;
      break;

    case WAIT_TRANSMIT_DONE:
      if (transmittedFlag) {
        transmittedFlag = false;
        if (transmissionState == RADIOLIB_ERR_NONE) {
          Serial.println(F("transmission finished!"));
        } else {
          Serial.print(F("failed, code "));
          Serial.println(transmissionState);
        }
        radio.finishTransmit();
        lastAction = millis();
        if (ackReceived) {
          masterState = ACK_RECEIVED;
        } else if (retryCount == 0) {
          masterState = WAIT_BEFORE_LISTEN;
        } else {
          masterState = SEND_LISTEN;
        }
      }
      break;

    case WAIT_BEFORE_LISTEN:
      if (millis() - lastAction >= 2000) { // Đợi 2s
        masterState = SEND_LISTEN;
      }
      break;

    case SEND_LISTEN:
      Serial.println("[TX] Sending 'tao muon nghe'...");
      transmissionState = radio.startTransmit("tao muon nghe");
      masterState = WAIT_ACK; // Đợi gửi xong mới chuyển sang nhận ACK
      lastAction = millis();
      break;

    case WAIT_ACK: {
        String ack;
        int state = radio.receive(ack, 2000); // Chờ tối đa 2s để nhận ACK
        if (state == RADIOLIB_ERR_NONE && ack == "ACK") {
          Serial.println("[TX] Received ACK!");
          ackReceived = true;
          retryCount = 0;
          masterState = ACK_RECEIVED;
        } else {
          Serial.println("[TX] No ACK, retrying...");
          retryCount++;
          if (retryCount < 5) {
            masterState = SEND_LISTEN;
          } else {
            Serial.println("[TX] Failed to get ACK after 5 tries.");
            masterState = SEND_MAIN;
            retryCount = 0;
          }
        }
      }
      break;

    case ACK_RECEIVED:
      // Nháy LED xanh 3 lần báo đã nhận ACK
      for (int i = 0; i < 3; i++) {
        strip.setPixelColor(0, strip.Color(0, 255, 0));
        strip.show();
        delay(200);
        strip.setPixelColor(0, strip.Color(0, 0, 0));
        strip.show();
        delay(200);
      }
      ackReceived = false;
      masterState = SEND_MAIN;
      break;
  }
}