#include <Adafruit_DotStar.h>
#include <bluefruit.h>
#include <InternalFileSystem.h>
#include "KeyService.h"
#include "KeyConfigService.h"
#include "KeyConfigLoader.h"
#include "ButtonManager.h"

#define SELECT_PIN PIN_BUTTON1

Adafruit_DotStar strip(1, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);
BLEDis bledis;
bool isConfigMode = false;
// uint8_t buttonPins[] = { 5, 19, 18, 17, 13, 12, 14, 15, 4, 16 };
uint8_t buttonPins[] = { 4 };

bool pinIsOn(uint8_t pin) {
    // Serial.print("Pin[");
    // Serial.print(pin);
    // Serial.print("] : ");
    bool isOn = digitalRead(pin) == 0;
    // Serial.print(isOn);
    // Serial.print("\n");
    return isOn;
}

ButtonManager buttonManager(buttonPins, sizeof(buttonPins), pinIsOn, sendKey);
KeyConfig keyConfig;

void setup() {
  strip.begin();
  strip.setBrightness(2);
  strip.setPixelColor(0, 255, 0, 0);
  strip.show();

  Serial.begin(115200);
  while (!Serial) delay(10);   // for nrf52840 with native usb
  
  pinMode(SELECT_PIN, INPUT_PULLUP);
  delay(1000);

  InternalFS.begin();

  isConfigMode = digitalRead(SELECT_PIN) == LOW;

  strip.begin();
  strip.setBrightness(2);
  if (isConfigMode) {
    Serial.println("config mode");
    strip.setPixelColor(0, 0, 0, 255);
  } else {
    Serial.println("key mode");
    strip.setPixelColor(0, 0, 255, 0);
  }
  strip.show();
  Serial.flush();

  keyConfig = KeyConfigLoader::load();
  buttonManager.setKeyConfig(keyConfig);

  Serial.println("config loaded");
  Serial.flush();
//
//  Bluefruit.begin();
//  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
//  Bluefruit.setName("SH-CON2");
//
//  // // Configure and Start Device Information Service
//  bledis.setManufacturer("FUZZILIA");
//  bledis.setModel("SH-CONTROLLER2");
//  bledis.begin();
//
//  startAdv();
}

void startAdv(void)
{  
  Serial.println("config startAdv");

  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  
  initKeyConfigService(keyConfig, !isConfigMode);
  if (!isConfigMode) {
    initKeyService();
  }

  Bluefruit.Advertising.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void loop() {
  delay(20);

  strip.begin();
  strip.setBrightness(2);
  if (digitalRead(SELECT_PIN) == LOW) {
    strip.setPixelColor(0, 0, 0, 255);
  } else {
    strip.setPixelColor(0, 0, 255, 0);
  }
  strip.show();

  if (!isConfigMode) {
      buttonManager.checkButtonState();
  }
}
