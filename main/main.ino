#include <Adafruit_MPR121.h>
#include <Adafruit_DotStar.h>
#include <bluefruit.h>
#include <InternalFileSystem.h>
#include <array>
#include "src/lib/SHController.h"
#include "CL3GD20.h"
#include "ConfigLoader.h"
#include "KeyService.h"
#include "KeyConfigService.h"
#include "ButtonMatrix.h"

Adafruit_DotStar strip(1, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);

CL3GD20 l2gd20; // ジャイロセンサー.

BLEDis bledis;

static SHController *sh_controller = nullptr;
static bool isConfigMode = false;

static float StickValue(int, TwoDimension dimension) {
  // analogRead(A3)
  // analogRead(A4)
  // if (joycon_packet) {
  //     auto stick_value = joycon_packet->readStickValue();
  //     switch (dimension) {
  //         case TwoDimension::X:
  //             return CapStickValue((float) (stick_value.horizontal - joycon_stick_center.horizontal) / 1900.0f);
  //         case TwoDimension::Y:
  //             return CapStickValue((float) (stick_value.vertical - joycon_stick_center.vertical) / 1900.0f);
  //     }
  // }
  return 0.0f;
}

std::vector<MotionSensorValue> MotionSensorValues() {
  return {
    MotionSensorValue{.time_span_s = 1.0f / 60 / 3, .gyro = {.x = 0.0f, .y = 0.0f, .z = 0.0f}},
    MotionSensorValue{.time_span_s = 1.0f / 60 / 3, .gyro = {.x = 0.0f, .y = 0.0f, .z = 0.0f}},
    MotionSensorValue{.time_span_s = 1.0f / 60 / 3, .gyro = {.x = 0.0f, .y = 0.0f, .z = 0.0f}},
  };
}

static void showMode(bool isConfigMode) {
  strip.begin();
  strip.setBrightness(2);
  if (isConfigMode)
  {
    Serial.println("config mode");
    strip.setPixelColor(0, 0, 0, 255);
  }
  else
  {
    Serial.println("key mode");
    strip.setPixelColor(0, 0, 255, 0);
  }
  strip.show();
  Serial.flush();
}

static void startAdv(void)
{
  if (!isConfigMode)
  {
    initKeyService();
  }
  else
  {
    initKeyConfigService();
  }

  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
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
  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
  Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
}

void setup() {
  strip.begin();
  strip.setBrightness(2);
  strip.setPixelColor(0, 255, 0, 0);
  strip.show();
  
	Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  analogReadResolution(12);
  InitPinsForButton();

  InternalFS.begin();
  isConfigMode = ConfigButtonIsOn();

  showMode(isConfigMode);

  auto config = ConfigLoader::load();
  sh_controller = new SHController(std::move(config), ButtonIsOn, StickValue, MotionSensorValues);

  Bluefruit.begin();
  Bluefruit.setTxPower(4); // Check bluefruit.h for supported values
  Bluefruit.setName("SH-CON2");

  // // Configure and Start Device Information Service
  bledis.setManufacturer("FUZZILIA");
  bledis.setModel("SH-CONTROLLER2");
  bledis.begin();

  startAdv();
}

void loop() {
  delay(1000);
}
