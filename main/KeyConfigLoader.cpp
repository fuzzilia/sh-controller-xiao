#include <Arduino.h>
#include <utility>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include "KeyConfigLoader.h"

using namespace Adafruit_LittleFS_Namespace;

const char *ApplicationKey = "SH-Config";
const char *ConfigKey = "config";
const char *CurrentKey = "current";

#define FILENAME "sh-config.conf"

File file(InternalFS);

void printBuffer(uint8_t *data, size_t size) {
    Serial.print("size=");
    Serial.print((unsigned int)size);
    Serial.print(" data=[");
    for (int i = 0; i < size; i++) {
        Serial.print(data[i]);
        Serial.print(" ");
    }
    Serial.println("]");
}

void KeyConfigLoader::save(uint8_t *data, size_t size) {
    if (file.open(FILENAME, FILE_O_WRITE)) {
        file.write(data, size);
        Serial.println("SaveConfig");
        printBuffer(data, size);
        file.close();
    } else {
        Serial.println("SaveFailed");
    }
}

KeyConfig KeyConfigLoader::load() {
    file.open(FILENAME, FILE_O_READ);
    if (file) {
        uint8_t buffer[KeyConfig::MAX_DATA_SIZE];
        file.read(buffer, sizeof(KeyConfig::MAX_DATA_SIZE));
        Serial.println("LoadConfig");
        printBuffer(buffer, KeyConfig::MAX_DATA_SIZE);
        file.close();
    } else {
        Serial.println("File not found.");
        return KeyConfig();
    }
}