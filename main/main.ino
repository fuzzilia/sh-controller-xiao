#include <Adafruit_MPR121.h>
#include <Adafruit_DotStar.h>
#include <bluefruit.h>
#include <InternalFileSystem.h>
#include <array>
#include "FreeRTOS.h"
#include "src/lib/SHController.h"
#include "CL3GD20.h"
#include "ConfigLoader.h"
#include "KeyService.h"
#include "KeyConfigService.h"
#include "ButtonMatrix.h"
#include "AnalogStick.h"

#define TICK_PER_SEC 60
#define READ_MOTION_PER_TICK 3

Adafruit_DotStar strip(1, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);

static CL3GD20 *l2gd20 = nullptr; // ジャイロセンサー.

BLEDis bledis;

TimerHandle_t tickTimer;
static int readMotionCount = 0;

static SHController *sh_controller = nullptr;
static bool isConfigMode = false;

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

void Tick(TimerHandle_t xTimer){
  // ジャイロセンサーの読み取り処理 + コントローラーの定期処理
  readMotionCount++;
  if (readMotionCount >= READ_MOTION_PER_TICK) {
    RefreshStickValue();
    sh_controller->tick();
    readMotionCount = 0;
  }
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
  InitPinsForStick();

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
  if (sh_controller->config().NeedsSensorInput()) {
    l2gd20 = new CL3GD20();
  }

  tickTimer = xTimerCreate("Tick", ms2tick(1000) / TICK_PER_SEC / READ_MOTION_PER_TICK, pdTRUE , 0, Tick);
}

void loop() {
  // こっちのloopではなにもしない。
  delay(10000);
}
