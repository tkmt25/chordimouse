#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <utils/debug.h>

using Condition = bool(*)();

/**
 * @brief 指定された条件を満たすもしくはタイムアウトまで待機
 * @param [in] condition  条件
 * @param [in] timeoutMs  タイムアウト(ms).0指定時はタイムアウトしない
 * @param [in] intervalMs インターバル(ms)
 * 
 * @retval true  条件を満たした
 * @retval false タイムアウトした
 */
inline bool waitCondition(Condition condition, const uint32_t timeoutMs=0, const uint32_t intervalMs=1)
{
    auto t0 = millis();

    while (!condition())
    {
        if (timeoutMs > 0 && millis() - t0 > timeoutMs) {
            DEBUG_PRINTF("ble timeouted %d %d", timeoutMs, millis() - t0);
            return false;
        }
        delay(intervalMs);
    }
    return true;
}