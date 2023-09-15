#include "FullColorLEDPattern.h"

FullColorLEDPattern::FullColorLEDPattern(
    const std::vector<FullColorLEDPatternItem> &items, int maxCount)
    : m_items(items), m_maxCount(maxCount)
{
}

std::tuple<bool, Color> FullColorLEDPattern::Tick()
{
    if (m_maxCount != 0 && m_currentCount >= m_maxCount)
    {
        return std::make_tuple(false, m_items.front().color);
    }
    auto result = GetColor();
    m_ticks++;
    if (m_ticks >= m_items.back().ticks)
    {
        m_ticks = 0;
        m_currentCount++;
    }
    return result;
}

std::tuple<bool, Color> FullColorLEDPattern::GetColor()
{
    if (m_ticks == 0)
    {
        return std::make_tuple(true, m_items.front().color);
    }
    else
    {
        for (int i = 0; i < m_items.size() - 1; i++)
        {
            if (m_items[i].ticks == m_ticks)
            {
                return std::make_tuple(true, m_items[i + 1].color);
            }
        }
    }
    return std::make_tuple(false, m_items.front().color);
}
