#include <Adafruit_NeoPixel.h> // RGB Led library

#define BUILDIN_RGB_LED_PIN 48

Adafruit_NeoPixel pixels(1, BUILDIN_RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

void setup () {  
  pixels.begin();

}

void loop () {

  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(150, 200, 0));
  pixels.show();
  delay(1000);

}