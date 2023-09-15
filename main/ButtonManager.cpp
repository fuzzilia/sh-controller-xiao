#include <array>
#include <Arduino.h>

static const std::array<uint8_t, 3> DirectButtonPins = {PIN_A1, PIN_A2, PIN_A3};
static const int CONFIG_BUTTON_INDEX = 0;

static int count = 0;

bool ButtonIsOn(int index)
{
    if (index < DirectButtonPins.size())
    {
        return digitalRead(DirectButtonPins[index]) == LOW;
    }
    else
    {
        return false;
    }
}

void InitButtons()
{
    for (const auto pin : DirectButtonPins)
    {
        pinMode(pin, INPUT_PULLUP);
    }
}

bool ConfigButtonIsOn()
{
    return ButtonIsOn(CONFIG_BUTTON_INDEX);
}