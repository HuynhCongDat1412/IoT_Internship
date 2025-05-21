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
const float carrier_frequency = 2450.0;
const float bandwidth = 1625.0;
const int spreading_factor = 7;
const int coding_rate = 5;
const uint8_t sync_word = 0x12;
const int output_power = 2;
const int preamble_length = 20;

enum State {
  SEND_MAIN,
  WAIT_LISTEN_MAIN,
  SEND_LISTEN,
  WAIT_TRANSMIT_DONE,
  WAIT_ACK,
  WAIT_SEND_MAIN_AGAIN,
  WAIT_LISTEN_ACK,
  ACK_RECEIVED,
  WAIT_BEFORE_LISTEN
};
State masterState = SEND_MAIN;
uint32_t lastAction = 0;
bool ackReceived = false; // Biến kiểm tra đã nhận ACK chưa

// save transmission state between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate that a packet was sent
volatile bool receivedFlag = false;
volatile bool transmittedFlag = false;
void SendData(String data);
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

void setRFlag(void) {
  // we got a packet, set the flag
  receivedFlag = true;
  // Serial.println("setFlag called!");
}

void checksettings(){  
  if (radio.setFrequency(carrier_frequency) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    Serial.println(F("Selected frequency is invalid for this module!"));
    while (true) { delay(10); }
  }
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


void setup() {
  Serial.begin(115200);
  delay(2000);
  strip.begin();
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.show();
  Serial.println("Initializing SX1280...");
  SPI.begin(36, 37, 35);
  // int state = radio.begin();
  int state = radio.begin(2450.0, 1625.0, 7, 5, 0x12, 2, 20);
  
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
  Serial.print(F("[SX1280] Sending first packet ... "));
  transmissionState = radio.startTransmit("Hello World!");
  checksettings();
  // set bandwidth to 203.125 kHz
  
}

String Recive(){
  String msg;int len = 0;
  if(receivedFlag==1) {
    // reset flag
    receivedFlag = false;
    int state = radio.readData(msg);    
    if (state == RADIOLIB_ERR_NONE) {
      if(msg != "" ){
        Serial.println("Data received: " + msg);
          strip.setPixelColor(0, strip.Color(0, 255, 0)); // LED xanh
          strip.show();
        if(msg == "need ACK"){
          // Gửi ACK lại cho master
          transmittedFlag = true;
          SendData("ACK");
        }
      }        
        else{
          Serial.println("Data received: NONE" + msg);
          strip.setPixelColor(0, strip.Color(255, 0, 0)); // LED đỏ
          strip.show();
        }      
    }
    else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
        // packet was received, but is malformed
        Serial.println(F("CRC error!"));
    } else {
      // some other error occurred
      // Serial.print(F("failed, code 3 "));
      // Serial.println(state);
    }
    // put module back to listen mode
    radio.startReceive();
    return msg;
  }
  return "NONE1";
}

void SendData(String data) {
  if(transmittedFlag) {
    // reset flag
    transmittedFlag = false;
    if (transmissionState == RADIOLIB_ERR_NONE) {
      // packet was successfully sent      
    } else {
      Serial.print(F("failed, code "));
      Serial.println(transmissionState);
    }
    radio.finishTransmit();     
    transmissionState = radio.startTransmit(data);

    if (transmissionState == RADIOLIB_ERR_NONE) {
      // packet was successfully sent
      Serial.println("[TX] Sending data. : " + data);
      Serial.println(F("transmission finished!"));
    } else {
      Serial.print(F("failed, code "));
      Serial.println(transmissionState);
  }
    strip.setPixelColor(0, strip.Color(255, 0, 0)); // LED đỏ
    strip.show();
  }
}
bool send = false;
String msg;
void loop() {
  static String mainMsg = " Hello";
  static int retryCount = 0;
  switch (masterState) {
    case SEND_MAIN:
      static long lastAction1 = 0;
      transmittedFlag = true;
      if (millis() - lastAction1 >= 500) { // Đợi 2s
        lastAction1 = millis();
        SendData(mainMsg);
      }
      static long lastAction2 = 0;
      if (millis() - lastAction2 >= 1500) { // Đợi 2s
        lastAction2 = millis();
        masterState = SEND_LISTEN;
      }
      //Serial.println("Sending main message: " + mainMsg);
      strip.setPixelColor(0, strip.Color(255, 0, 0)); // LED đỏ
      strip.show();
      break;
    case SEND_LISTEN:
      transmittedFlag = true;
      SendData("need ACK");
      masterState = WAIT_TRANSMIT_DONE;
      break;
    case WAIT_TRANSMIT_DONE:
      if (transmittedFlag) {  
        transmittedFlag = false;
        radio.finishTransmit();
        radio.startReceive(); // Chỉ gọi sau khi truyền xong!
        masterState = WAIT_LISTEN_ACK;
      }
      break;
    case WAIT_LISTEN_ACK:
      //receivedFlag = true;
      static long lastAction4 = 0;

      receivedFlag = true;
      msg = Recive();
      if(millis() - lastAction4 >= 3000) {
        lastAction4 = millis();
        Serial.println("Waiting for ACK...");
        Serial.println("Received: " + msg);
      }
      static long lastAction3 = 0;
      if (msg == "ACK") {
        Serial.println("ACK received");
        ackReceived = true;
        masterState = ACK_RECEIVED;
      } 
      else if (millis() - lastAction3 >= 1000) {
        lastAction3 = millis();
        transmittedFlag = true;
        //masterState = SEND_MAIN;
      }
      break;
    case ACK_RECEIVED:
      strip.setPixelColor(0, strip.Color(0, 255, 0)); // LED xanh
      strip.show();
      // Gửi lại dữ liệu
      break;
  }
}