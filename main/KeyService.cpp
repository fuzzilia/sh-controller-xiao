#include <Arduino.h>
#include <bluefruit.h>
#include "KeyService.h"

BLEHidAdafruit blehid;

void initKeyService()
{
    blehid.begin();
    Bluefruit.Advertising.addService(blehid);
}

hid_keyboard_report_t report;

void setKeyToReport(const KeyboardValue &key)
{
    varclr(&report);
    report.keycode[0] = key.keyCode();
    report.modifier = key.modifier();
}

void printSendKey(const KeyboardValue &key)
{
    Serial.print("send key");
    if (key.hasCtrl())
    {
        Serial.print("[ctrl]");
    }
    if (key.hasShift())
    {
        Serial.print("[shift]");
    }
    if (key.hasAlt())
    {
        Serial.print("[alt]");
    }
    if (key.hasGui())
    {
        Serial.print("[gui]");
    }
    Serial.print(key.keyCode());
    Serial.print("\n");
}

void sendKey(KeyboardValue key)
{
    printSendKey(key);
    setKeyToReport(key);
    blehid.keyboardReport(BLE_CONN_HANDLE_INVALID, &report);
}
