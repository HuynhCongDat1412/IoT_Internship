#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include <RadioLib.h>
#include <Adafruit_NeoPixel.h>

Module mod(39, 42, 40, 41, SPI);
SX1280 radio(&mod);
const float carrier_frequency = 2450.0;
const float bandwidth = 1625.0;
const int spreading_factor = 7;
const int coding_rate = 5;
const uint8_t sync_word = 0x12;
const int output_power = 2;
const int preamble_length = 20;
// Khởi tạo cho các tham số RF
uint32_t count = 0; // Biến đếm gửi tin nhắn
#define LED_PIN 48
#define NUM_LEDS 1
int transmissionState = RADIOLIB_ERR_NONE;  //trạng thái truyền/ nhận tin nhắn
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
// flag to indicate that a packet was received
volatile bool receivedFlag = false;
volatile bool transmittedFlag = false;
static bool mainFlag = false;
static bool acktransmitFlag = false;
// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif

// Khởi tạo màn hình OLED (I2C), SDA = 7, SCL = 15
SSD1306Wire display(0x3C, 7, 15);
void setFlag(void) {
  receivedFlag = true;
}
String Msg = ""; // Lưu tin nhắn trước đó
String lastMsg = ""; // Lưu tin nhắn trước đó

void SendData(String data);
void setup() {
  Serial.begin(115200);
  delay(2000);

  //------------------------Khoi tao RF
  SPI.begin(36, 37, 35);

  int state = radio.begin(2450.0, 1625.0, 7, 5, 0x12, 2, 20);  // cấu hình thông số RF
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

  Serial.println(F("All settings succesfully changed!"));
  //-----------------------------------------------------------khoi tao led
  display.init();
  display.clear();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);   
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.show(); 
}

void monitor(int State) {
  // Print the current state of the radio
  Serial.print("Radio State: ");
  Serial.println(State);
  Serial.print("Carrier Frequency: ");
  Serial.print(carrier_frequency);
  Serial.println(" MHz");
  Serial.print("Bandwidth: ");
  Serial.print(bandwidth);
  Serial.println(" kHz");
  Serial.print("Spreading Factor: ");
  Serial.println(spreading_factor);
  Serial.print("Coding Rate: ");
  Serial.println(coding_rate);
  Serial.print("Sync Word: ");
  Serial.println(sync_word, HEX);
}

void showOled(String str) {  
    display.clear();
    // Hiển thị thông số RF ở dòng đầu tiên
    String rfInfo = "F:" + String(carrier_frequency, 1) + "MHz ";
    //rfInfo += "BW:" + String(bandwidth, 1) + "kHz ";
    //rfInfo += "SF:" + String(spreading_factor);
    //rfInfo += " CR:" + String(coding_rate);
    rfInfo += " SW:0x" + String(sync_word, HEX);
    //rfInfo += " P:" + String(output_power) + "dBm";
    display.drawString(0, 0, rfInfo);
    display.drawString(0, 12, "Data: " + str);
    display.drawString(0, 24, "RSSI: " + String(radio.getRSSI(), 1) + " dBm");
    display.drawString(0, 36, "SNR: " + String(radio.getSNR(), 1) + " dB");
    display.drawString(0, 48, "FreqErr: " + String(radio.getFrequencyError(), 1) + " Hz");
    display.display();
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
        showOled(msg);        
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
      Serial.print(F("failed, code "));
      Serial.println(state);
    }
    return msg; // trả về tin nhắn đã nhận
  }
  return ""; // nếu không có tin nhắn nào được nhận, trả về chuỗi rỗng
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

void loop() {
  if(!mainFlag){
    Msg = Recive();
    if (Msg == " Hello")
      {
        mainFlag = true; //Nếu đã nhận được tin nhắn main, chuyển sang chế độ gửi "ACK"
      }
  }
  if (mainFlag){
    int state = radio.transmit("ACK");
    Serial.println("transmit again");
  }
}
