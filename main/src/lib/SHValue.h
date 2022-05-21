#ifndef SH_JOYCON_SHVALUE_H
#define SH_JOYCON_SHVALUE_H

#include <stdint.h>

template<class T>
struct PositiveAndNegative {
    T positive;
    T negative;

    static PositiveAndNegative<T> zero() {
        return {.positive = 0, .negative = 0};
    }
};

template<class T>
struct ThreeDimensionValue {
    T x;
    T y;
    T z;
};

enum class TwoDimension : uint8_t {
    X,
    Y,
};

enum class ThreeDimension : uint8_t {
    X,
    Y,
    Z,
};

enum class NullableThreeDimension : uint8_t {
    Null,
    X,
    Y,
    Z,
};

#endif //SH_JOYCON_SHVALUE_H
