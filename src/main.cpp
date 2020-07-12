#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>  // Graphics and font library for ST7735 driver chip
#include <Wire.h>

#define TP_PIN_PIN 33
#define TP_PWR_PIN 25
#define LED_PIN 4

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

uint8_t func_select = 0;
boolean pressed = false;
uint8_t MAX_SELECT_MODE_COUNT = 2;

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);

  pinMode(TP_PIN_PIN, INPUT);
  pinMode(TP_PWR_PIN, PULLUP);  //! Must be set to pull-up output mode in order
                                //! to wake up in deep sleep mode
  digitalWrite(TP_PWR_PIN, HIGH);
}

void SleepMode() {
  tft.fillScreen(TFT_BLACK);
  tft.writecommand(ST7735_SLPIN);
  tft.writecommand(ST7735_DISPOFF);
  esp_sleep_enable_ext1_wakeup(GPIO_SEL_33, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_deep_sleep_start();
}

void loop() {
  if (digitalRead(TP_PIN_PIN) == HIGH) {
    if (!pressed) {
      tft.fillScreen(TFT_BLACK);
      func_select =
          func_select + 1 > MAX_SELECT_MODE_COUNT ? 0 : func_select + 1;
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      pressed = true;
    }
  } else {
    pressed = false;
  }

  switch (func_select) {
    case 0:
      tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("HOGE", tft.width() / 2, tft.height() / 2, 4);
      break;
    case 1:
      tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("PIYO", tft.width() / 2, tft.height() / 2, 4);
      break;
    case 2:
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Press again to wake up", tft.width() / 2,
                     tft.height() / 2);
      delay(500);
      SleepMode();
      break;
    default:
      break;
  }
}