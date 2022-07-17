#ifndef SH_CONTROLLER_LED_INDICATOR_H
#define SH_CONTROLLER_LED_INDICATOR_H

#include <vector>
#include <array>

class LEDIndicator {
    int m_pin;
    int m_endressBlinkPeriodTickCount = 0;
    int m_singleBlinkPeriodTickCount = 0;
    int m_singleBlinkCount = 0;
    int m_singleBlinkStartTickCount = -1;
    std::array<bool, 4> m_enabledEndressBlinkBrightPattern;

public:
    enum class EndressBlinkPattern : uint16_t {
        Long = 0,
        Short = 1,
        Full = 2,
        Wave = 3,
    };

    LEDIndicator(int pin);
    void beginEndressBlink(int periodTickCount);
    void enableEndressBlinkBrightPattern(EndressBlinkPattern patternNumber);
    void disableEndressBlinkBrightPattern(EndressBlinkPattern patternNumber);
    void initSingleBlink(int periodTickCount);
    void addSingleBlink(int count);
    void tick(int tickCount);
};

#endif // SH_CONTROLLER_LED_INDICATOR_H
