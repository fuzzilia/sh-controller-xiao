#ifndef SH_CONTROLLER_FULL_COLOR_LED_PATTERN_H
#define SH_CONTROLLER_FULL_COLOR_LED_PATTERN_H

#include <array>
#include <vector>
#include <tuple>

struct Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct FullColorLEDPatternItem
{
    unsigned int ticks;
    Color color;
};

class FullColorLEDPattern
{
private:
    std::vector<FullColorLEDPatternItem> m_items;
    int m_maxCount;
    int m_ticks = 0;
    int m_currentCount = 0;

    std::tuple<bool, Color> GetColor();

public:
    FullColorLEDPattern(const std::vector<FullColorLEDPatternItem> &items, int maxCount);
    std::tuple<bool, Color> Tick();
};

#endif // SH_CONTROLLER_FULL_COLOR_LED_PATTERN_H
