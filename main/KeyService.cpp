#include <Arduino.h>
#include <bluefruit.h>
#include "KeyService.h"

BLEHidAdafruit blehid;

void initKeyService() {
  blehid.begin();
  Bluefruit.Advertising.addService(blehid);
}

hid_keyboard_report_t report;

void setKeyToReport(Key key) {
    report.keycode[0] = key.key;
    report.modifier = 
        (key.modifer.control << 0) |
        (key.modifer.shift << 1) |
        (key.modifer.alt << 2) |
        (key.modifer.gui << 3);
}

void printSendKey(Key key) {
    Serial.print("send key");
    if (key.modifer.control) { Serial.print("[control]"); }
    if (key.modifer.shift) { Serial.print("[shift]"); }
    if (key.modifer.alt) { Serial.print("[alt]"); }
    if (key.modifer.gui) { Serial.print("[gui]"); }
    Serial.print(key.key);
    Serial.print("\n");
}

void sendKey(Key key) {
    printSendKey(key);
    setKeyToReport(key);
    blehid.keyboardReport(BLE_CONN_HANDLE_INVALID, &report);
}
