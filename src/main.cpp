#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>  // Graphics and font library for ST7735 driver chip
#include <Wire.h>
#include <pcf8563.h>

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define TP_PIN_PIN 33
#define TP_PWR_PIN 25
#define LED_PIN 4

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
PCF8563_Class rtc;

boolean pressed = false;
uint8_t func_select = 0;
uint8_t MAX_SELECT_MODE_COUNT = 2;
RTC_Date datetime;

void setupRTC() {
  rtc.begin(Wire);
  rtc.check();  // Check if the RTC clock matches, if not, use compile time

  datetime = rtc.getDateTime();
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setSwapBytes(true);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);

  setupRTC();

  pinMode(TP_PIN_PIN, INPUT);
  pinMode(TP_PWR_PIN, PULLUP);  //! Must be set to pull-up output mode in order
                                //! to wake up in deep sleep mode
  digitalWrite(TP_PWR_PIN, HIGH);
}

void showTime() {
  datetime = rtc.getDateTime();

  // show hour
  tft.setRotation(0);  // button exist down direction
  tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  if (datetime.hour < 10) {
    tft.drawString("0" + String(datetime.hour), 9, 16, 7);
  } else {
    tft.drawString(String(datetime.hour), 9, 16, 7);
  }

  // show colon
  tft.setRotation(1);  // horizonal
  if (datetime.second % 2) {
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
  } else {
    tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  }
  tft.drawString(":", tft.width() / 2 - 8, tft.height() / 2 - 24, 7);

  // show minute
  tft.setRotation(0);  // button exist down direction
  tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  if (datetime.minute < 10) {
    tft.drawString("0" + String(datetime.minute), 10, 90, 7);
  } else {
    tft.drawString(String(datetime.minute), 10, 90, 7);
  }
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
      showTime();
      break;
    case 1:
      tft.setRotation(2);
      tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("PIYO", tft.width() / 2, tft.height() / 2, 4);
      break;
    case 2:
      tft.setRotation(0);
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