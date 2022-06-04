#include <Arduino.h>
#include <utility>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include "ConfigLoader.h"

using namespace Adafruit_LittleFS_Namespace;

#define FILENAME "sh-config.conf"

File file(InternalFS);
static uint8_t buffer[512] = {0};
static bool loaded = false;

void printBuffer(uint8_t *data, size_t size)
{
    Serial.print("size=");
    Serial.print((unsigned int)size);
    Serial.print(" data=[");
    for (int i = 0; i < size; i++)
    {
        Serial.print(data[i]);
        Serial.print(" ");
    }
    Serial.println("]");
}

void ConfigLoader::save(uint8_t *data, size_t size)
{
    if (file.open(FILENAME, FILE_O_WRITE))
    {
        file.write(data, size);
        Serial.println("SaveConfig");
        Serial.flush();
        printBuffer(data, size);
        file.close();
        memcpy(buffer, data, size);
    }
    else
    {
        Serial.println("SaveFailed");
        Serial.flush();
    }
}

std::unique_ptr<SHConfig> ConfigLoader::load()
{
    auto buffer = loadRaw();
    if (buffer) {
        return std::unique_ptr<SHConfig>(new SHConfig(buffer));
    } else {
        return nullptr;
    }
}

const uint8_t *ConfigLoader::loadRaw()
{
    if (loaded) {
        return buffer;
    }
    file.open(FILENAME, FILE_O_READ);
    if (file)
    {
        file.read(buffer, sizeof(buffer));
        file.close();
        loaded = true;
        // Serial.println("LoadConfig");
        // Serial.flush();
        // printBuffer(buffer, sizeof(buffer));
        // Serial.flush();
        return buffer;
    }
    else
    {
        return nullptr;
    }

}