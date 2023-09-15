#ifndef SH_CONTROLLER_FULL_COLOR_LED_INDICATOR_H
#define SH_CONTROLLER_FULL_COLOR_LED_INDICATOR_H

#include <array>
#include <memory>
#include "FullColorLEDPattern.h"

class FullColorLEDIndicator
{
    enum class Mode
    {
        None = 0,
        Standard = 1,
        Config = 2,
    };

private:
    std::array<uint8_t, 3> m_pins;
    std::unique_ptr<FullColorLEDPattern> m_pattern;
    Mode m_mode = Mode::None;

public:
    FullColorLEDIndicator(const std::array<uint8_t, 3> &pins);
    void InitPins();
    void StartConfigMode();
    void StartStandardMode();
    void Connected();
    void Disconnected();
    void Tick();
};

#endif // SH_CONTROLLER_FULL_COLOR_LED_INDICATOR_H
