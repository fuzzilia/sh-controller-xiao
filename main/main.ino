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
#include "nrf_rng.h"
#include "nrf_soc.h"

#define TICK_PER_SEC 60
#define READ_MOTION_PER_TICK 3

Adafruit_DotStar strip(1, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);

// static CL3GD20 *l2gd20 = nullptr; // ジャイロセンサー.

BLEDis bledis;

TimerHandle_t tickTimer;
static int readMotionCount = 0;

static SHController *sh_controller = nullptr;
static bool isConfigMode = false;

std::vector<MotionSensorValue> MotionSensorValues()
{
  return {
      MotionSensorValue{.time_span_s = 1.0f / 60 / 3, .gyro = {.x = 0.0f, .y = 0.0f, .z = 0.0f}},
      MotionSensorValue{.time_span_s = 1.0f / 60 / 3, .gyro = {.x = 0.0f, .y = 0.0f, .z = 0.0f}},
      MotionSensorValue{.time_span_s = 1.0f / 60 / 3, .gyro = {.x = 0.0f, .y = 0.0f, .z = 0.0f}},
  };
}

static void GenerateRandomAddress(ble_gap_addr_t &addr)
{
  addr.addr_id_peer = 0;
  addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
  for (int i = 0; i < BLE_GAP_ADDR_LEN; i++) {
    addr.addr[i] = 0;
  }

  uint8_t availableBytes;
  do {
    Serial.println("generating...");
    Serial.flush();
    delay(10);
    sd_rand_application_bytes_available_get(&availableBytes);
  } while (availableBytes < BLE_GAP_ADDR_LEN);
  sd_rand_application_vector_get(addr.addr, BLE_GAP_ADDR_LEN);
  Serial.println("generated.");
  for (int i = 0; i < BLE_GAP_ADDR_LEN; i++) {
    Serial.print(i);
    Serial.print(" : ");
    Serial.println(addr.addr[i]);
  }
  Serial.flush();
}

static void showMode(bool isConfigMode)
{
  if (isConfigMode)
  {
    Serial.println("config mode");
    strip.setPixelColor(0, 0, 0, 20);
  }
  else
  {
    Serial.println("key mode");
    strip.setPixelColor(0, 0, 20, 0);
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
    ble_gap_addr_t addr;
    GenerateRandomAddress(addr);
    Bluefruit.setAddr(&addr);

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

void Tick(TimerHandle_t xTimer)
{
  // ジャイロセンサーの読み取り処理 + コントローラーの定期処理
  readMotionCount++;
  Serial.println("tick");
  Serial.flush();
  if (readMotionCount >= READ_MOTION_PER_TICK)
  {
    RefreshStickValue();
    sh_controller->tick();
    readMotionCount = 0;
  }
}

void setup()
{
  strip.begin();
  strip.setBrightness(2);
  strip.show();
  Serial.begin(115200);
  while (!Serial)
    delay(10); // for nrf52840 with native usb

  analogReadResolution(12);
  InitPinsForButton();
  InitPinsForStick();

  InternalFS.begin();
  strip.setPixelColor(0, 255, 255, 0);
  strip.show();
  isConfigMode = ConfigButtonIsOn();

  showMode(isConfigMode);

  Serial.println("start load");
  Serial.flush();
  auto config = ConfigLoader::load();
  {
    auto configString = config->ToString();
    Serial.println(configString.c_str());
    Serial.flush();
  }
  sh_controller = new SHController(std::move(config), ButtonIsOn, StickValue, MotionSensorValues);

  Bluefruit.begin();
  Bluefruit.setTxPower(4); // Check bluefruit.h for supported values
  Bluefruit.setName("SH-CON2");

  // // Configure and Start Device Information Service
  bledis.setManufacturer("FUZZILIA");
  bledis.setModel("SH-CONTROLLER2");
  bledis.begin();

  startAdv();
  if (sh_controller->config().NeedsSensorInput())
  {
    // l2gd20 = new CL3GD20();
  }

  tickTimer = xTimerCreate("Tick", ms2tick(1000) / TICK_PER_SEC / READ_MOTION_PER_TICK, pdTRUE, 0, Tick);

  Serial.println("start");
  Serial.flush();
}

uint8_t val = 0;

void loop()
{
  // こっちのloopではなにもしない。
  delay(10000);
}
