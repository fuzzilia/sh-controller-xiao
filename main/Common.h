#ifndef SH_CONTROLLER_COMMON_H
#define SH_CONTROLLER_COMMON_H

#ifdef SH_CONTROLLER_PRODUCTION
#define DEBUG_PRINT(str)
#else
#define DEBUG_PRINT(str)     \
    {                        \
        Serial.println(str); \
        Serial.flush();      \
    }
#endif

#endif // SH_CONTROLLER_COMMON_H