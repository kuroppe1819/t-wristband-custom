#include "esp_adc_cal.h"
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <Wire.h>
#include <github_logo.h>
#include <pcf8563.h>
#include <qr_github.h>
#include <qr_twitter.h>
#include <twitter_logo.h>

#define BATT_ADC_PIN 35
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define TP_PIN_PIN 33
#define TP_PWR_PIN 25
#define LED_PIN 4

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
PCF8563_Class rtc;

boolean pressed = false;
int DEFAULT_VREF = 1100;
uint8_t func_select = 0;
uint8_t MAX_SELECT_MODE_COUNT = 3;
RTC_Date datetime;

void setupRTC()
{
    rtc.begin(Wire);
    rtc.check(); // Check if the RTC clock matches, if not, use compile time

    datetime = rtc.getDateTime();
}

void setup()
{
    Serial.begin(115200);

    tft.init();
    tft.setSwapBytes(true);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000);

    setupRTC();

    pinMode(TP_PIN_PIN, INPUT);
    pinMode(TP_PWR_PIN, PULLUP); //! Must be set to pull-up output mode in order
        //! to wake up in deep sleep mode
    digitalWrite(TP_PWR_PIN, HIGH);
}

float getVoltage()
{
    uint16_t v = analogRead(BATT_ADC_PIN);
    return ((float)v / 4095.0) * 2.0 * 3.3 * (DEFAULT_VREF / 1000.0);
}

void showTime(int pos_x, int pos_y)
{
    datetime = rtc.getDateTime();

    // show hour
    tft.setRotation(0); // button exist down direction
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    if (datetime.hour < 10) {
        tft.drawString("0" + String(datetime.hour), pos_x, pos_y, 7);
    } else {
        tft.drawString(String(datetime.hour), pos_x, pos_y, 7);
    }

    // show colon
    tft.setRotation(1); // horizonal
    if (datetime.second % 2) {
        tft.setTextColor(TFT_BLACK, TFT_BLACK);
    } else {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
    }
    tft.drawString(":", tft.width() / 2, tft.height() / 2 - pos_y, 7);

    // show minute
    tft.setRotation(0); // button exist down direction
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    if (datetime.minute < 10) {
        tft.drawString("0" + String(datetime.minute), pos_x, pos_y + 74, 7);
    } else {
        tft.drawString(String(datetime.minute), pos_x, pos_y + 74, 7);
    }
}

void showBatteryCharge(int pos_x, int pos_y)
{
    // draw battery charge
    int width = 16;
    int height = 8;
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawRect(pos_x, pos_y, width, height, TFT_GREEN);
    tft.fillRect(pos_x + width, pos_y + 2, 1, 4, TFT_GREEN);

    float voltage = getVoltage();
    if (voltage >= 4.0) {
        tft.fillRect(pos_x, pos_y, width, height, TFT_GREEN);
    } else if (4.0 > voltage && voltage >= 3.0) {
        tft.fillRect(pos_x, pos_y, width - width / 4, height, TFT_GREEN);
    } else if (3.0 > voltage && voltage >= 2.0) {
        tft.fillRect(pos_x, pos_y, width - width / 4 * 2, height, TFT_GREEN);
    } else if (2.0 > voltage && voltage >= 1.0) {
        tft.fillRect(pos_x, pos_y, width - width / 4 * 3, height, TFT_GREEN);
    } else {
        tft.fillRect(pos_x, pos_y, 0, height, TFT_GREEN);
    };
}

void SleepMode()
{
    tft.fillScreen(TFT_BLACK);
    tft.writecommand(ST7735_SLPIN);
    tft.writecommand(ST7735_DISPOFF);
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_33, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_deep_sleep_start();
}

void loop()
{
    if (digitalRead(TP_PIN_PIN) == HIGH) {
        if (!pressed) {
            tft.fillScreen(TFT_BLACK);
            func_select = func_select + 1 > MAX_SELECT_MODE_COUNT ? 0 : func_select + 1;
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
        showBatteryCharge(9, 4);
        showTime(9, 24);
        break;
    case 1:
        tft.setRotation(2);
        tft.setBitmapColor(TFT_WHITE, TFT_WHITE);
        tft.pushImage(0, 0, 80, 80, twitter_logo);
        tft.pushImage(0, 80, 80, 80, qr_twitter);
        break;
    case 2:
        tft.setRotation(2);
        tft.setBitmapColor(TFT_WHITE, TFT_WHITE);
        tft.pushImage(0, 0, 80, 80, github_logo);
        tft.pushImage(0, 80, 80, 80, qr_github);
        break;
    case 3:
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