#include "Input.h"
#include <array>
#include <Arduino.h>
#include <Wire.h>

static const std::array<uint8_t, 3> DirectButtonPins = {D6, D7, D8};
static const int CONFIG_BUTTON_INDEX = 0;
static const byte PAD_BORD_ADDRESS = 0x19;
static const int PAD_PACKET_SIZE = 12;
static const int PAD_BUTTON_COUNT = 7;
static const uint16_t STICK_CENTER_VALUE = 0x0200;
static const uint16_t STICK_MAX_VALUE = 0x0388;
static const uint16_t STICK_MIN_VALUE = 0x0088;

////// stored pad state
static byte padButtonState = 0;
static uint16_t stickX = STICK_CENTER_VALUE;
static uint16_t stickY = STICK_CENTER_VALUE;

static float AnalogValueToFloat(uint16_t uintValue)
{
    if (uintValue >= STICK_MAX_VALUE)
    {
        return 1.0f;
    }
    if (uintValue <= STICK_MIN_VALUE)
    {
        return -1.0f;
    }
    int32_t intValue = uintValue - STICK_CENTER_VALUE;
    return ((float)intValue / ((float)(STICK_MAX_VALUE - STICK_MIN_VALUE) / 2));
}

float StickValue(int, TwoDimension dimension)
{
    switch (dimension)
    {
    case TwoDimension::X:
        return AnalogValueToFloat(stickX);
    case TwoDimension::Y:
        return AnalogValueToFloat(stickY);
    default:
        return 0.0f;
    }
}

bool ButtonIsOn(int index)
{
    if (index < DirectButtonPins.size())
    {
        return digitalRead(DirectButtonPins[index]) == LOW;
    }
    else
    {
        int padButtonIndex = index - DirectButtonPins.size();
        return (padButtonState & (0x01 << padButtonIndex)) != 0;
    }
}

void InitializeInput()
{
    for (const auto pin : DirectButtonPins)
    {
        pinMode(pin, INPUT_PULLUP);
    }
    Wire.begin();
}

void RefreshInput()
{
    Wire.requestFrom(PAD_BORD_ADDRESS, PAD_PACKET_SIZE);
    padButtonState = 0;

    if (Wire.available() == PAD_PACKET_SIZE)
    {
        Wire.read();
        for (int i = 0; i < PAD_BUTTON_COUNT; i++)
        {
            padButtonState |= Wire.read() << i;
        }
        stickX = (Wire.read() << 8) | Wire.read();
        stickY = (Wire.read() << 8) | Wire.read();
    }
    else
    {
        stickX = STICK_CENTER_VALUE;
        stickY = STICK_CENTER_VALUE;
    }
}

bool ConfigButtonIsOn()
{
    return ButtonIsOn(CONFIG_BUTTON_INDEX);
}