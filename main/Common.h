#ifdef SH_CONTROLLER_PRODUCTION
#define DEBUG_PRINT(str)
#else
#define DEBUG_PRINT(str)     \
    {                        \
        Serial.println(str); \
        Serial.flush();      \
    }
#endif