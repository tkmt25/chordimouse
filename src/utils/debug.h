/**
 * @brief デバッグ用ユーティリティ
 */
#pragma once

#include <Arduino.h>
#include <stdio.h>

#define __DEBUG__ (0)

#if __DEBUG__

#include <string.h>

inline void debugInit()
{
    Serial.begin(115200);

#if 1
    while (!Serial) {
        delay(10);
    }
#else
    delay(100);
#endif
}

inline void debugEnd()
{
    Serial.flush();
    Serial.end();
    while (Serial) {
        delay(10);
    }
}

template <typename... Args>
inline void _debugPrintf(const char* fmt, Args... args)
{
    char buf[128];
    snprintf(buf, sizeof(buf), fmt, args...);
    Serial.println(buf);
}

template <typename... Args>
inline void _debugPrintfAligned(const char* file, const char* func, int line, const char* fmt, Args... args)
{
    char label[64];
    snprintf(label, sizeof(label), "%s::%s(%d)", file, func, line);

    char buf[512];
    snprintf(buf, sizeof(buf), fmt, args...);

    Serial.printf("%08lu : %-50s : %s\n", millis(), label, buf);
}

class _stopwatch_ms
{
public:
    _stopwatch_ms(const char* fileName, const char* functionName, const int line, const char* label=""): fileName(fileName), functionName(functionName), line(line), label(label)
    {
        t0 = millis();
    }
    ~_stopwatch_ms()
    {
        auto now = millis();
        _debugPrintfAligned(fileName, functionName, line, "<< %s %dms >>", label, now - t0);
    }

private:
    uint32_t t0;
    const char* fileName;
    const char* functionName;
    const int line;
    const char* label;
};


#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define stopwatch_ms(...)          _stopwatch_ms __ss##__LINE__(__FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DEBUG_PRINTF(fmt, ...)  _debugPrintfAligned(__FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#else

#define debugInit()
#define debugEnd()
#define debugPrintf(...) 
#define stopwatch_ms(...) 
#define DEBUG_PRINTF(...) 

#endif