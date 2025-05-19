#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// put function declarations here:
#define LED_PIN 48
#define NUM_LEDS 1

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  strip.begin();
    strip.show(); // Khởi tạo LED
}

void loop() {
  // put your main code here, to run repeatedly:
  strip.setPixelColor(0, strip.Color(255, 0, 0)); // Đỏ
  strip.show();
  delay(1000);

  strip.setPixelColor(0, strip.Color(0, 255, 0)); // Xanh lá
  strip.show();
  delay(1000);

  strip.setPixelColor(0, strip.Color(0, 0, 255)); // Xanh dương
  strip.show();
  delay(1000);
}

