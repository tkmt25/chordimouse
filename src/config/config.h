#pragma once

#include <stdint.h>
#include <utils/serializable.h>
#include <utils/internal_fs.h>
#include <utils/debug.h>

#define ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD 1.0
#define ARDUINOJSON_NEGATIVE_EXPONENTIATION_THRESHOLD 0.001
#include <ArduinoJson.h>

/*
設定値について
- ジョイスティックの不感帯
- lightsleep/deepsleep時間
- マウス感度(MICKEY/Gain)

 */


/**
 * @brief グローバル設定を保持するクラス。設定値はFlashから読み込む
 *
 */
class config : public serializable<config>
{
public:
    config() = default;

    /**
     * @brief 同時押しの待機時間
     */
    uint32_t inline getChordScanTimeoutMs() const
    {
        return _chordScanTimeoutMs;
        //return 100;
    }

    /**
     * @brief ジョイスティックX軸の不感帯
     */
    uint32_t inline getJoystickXDeadband() const
    {
        return _joystickXDeadband;
    }

    /**
     * @brief ジョイスティックY軸の不感帯
     */
    uint32_t inline getJoystickYDeadband() const
    {
        return _joystickYDeadband;
    }

    /**
     * @brief ディープスリープに移行するまでの時間
     */
    uint32_t inline getDeepSleepTimeoutMs() const
    {
        return _deepSleepTimeoutMs;
        //return 30 * 60 * 1000; // 30 min
    }

    float inline getMickeyScale() const
    {
        return _mickeyScale;
    }

    float inline getMouseNegativeGain() const
    {
        return _mouseNegativeGain;
    }

    /**
     * @brief ライトスリープに移行するまでの時間
     */
    uint32_t inline getLightSleepTimeoutMs() const
    {
        //return 30 * 1000; // 30sec
        return _lightSleepTimeoutMs;
    }

    int8_t inline getTxPower() const
    {
        return _txPower;
    }

    uint16_t inline getConnectionIntervalMin() const
    {
        return _connectionIntervalMin;
    }

    uint16_t inline getConnectionIntervalMax() const
    {
        return _connectionIntervalMax;
    }

    uint32_t inline getMouseReportIntervalMs() const
    {
        return _mouseReportIntervalMs;
    }

    
    uint16_t inline serialize(uint8_t *buffer) const
    {
        StaticJsonDocument<BUFSIZE> doc;
        toJson(doc.to<JsonVariant>());
        auto size = measureJson(doc);
        serializeJson(doc, buffer, size);
        return size;
    }

    bool inline deserialize(const uint8_t *buffer, const uint16_t buffSize)
    {
        StaticJsonDocument<BUFSIZE> doc;

        auto err = deserializeJson(doc, buffer, buffSize);
        if (err) {
            DEBUG_PRINTF("deserialze error %d", err.code());
            return false;
        }

        fromJson(doc.as<JsonVariantConst>());
        return true;
    }


    uint16_t inline getSerializedSize() const
    {
        StaticJsonDocument<BUFSIZE> doc;
        toJson(doc.to<JsonVariant>());
        return measureJson(doc);
    }

    

    constexpr static int MAX_BUFFSIZE = 512;

private:
    constexpr static int CONFIG_VERSION = 1;
    constexpr static int BUFSIZE = 512; /// < 設定が増えてきたら調整

    uint32_t _version = CONFIG_VERSION;
    uint32_t _chordScanTimeoutMs = 150;
    uint32_t _deepSleepTimeoutMs = 30 * 60 * 1000;
    uint32_t _lightSleepTimeoutMs = 30 * 1000;
    uint8_t _joystickXDeadband = 5;
    uint8_t _joystickYDeadband = 5;
    uint8_t _mouseNegativeGain = 2;
    int8_t _txPower = 4;
    uint16_t _connectionIntervalMin = 6;
    uint16_t _connectionIntervalMax = 9;
    uint32_t _mouseReportIntervalMs = 10;
    float  _mickeyScale = 0.045f;

    void toJson(JsonVariant j) const
    {
        j["version"] = CONFIG_VERSION;
        j["chord_timeout"] = _chordScanTimeoutMs;
        j["deepsleep_timeout"] = _deepSleepTimeoutMs;
        j["lightsleep_timeout"] = _lightSleepTimeoutMs;
        j["joy_x_deadband"] = _joystickXDeadband;
        j["joy_y_deadband"] = _joystickYDeadband;
        j["mouse_negative_gain"] = _mouseNegativeGain;
        j["mickey_scale"] = _mickeyScale;
        j["tx_power"] = _txPower;
        j["conn_interval_min"] = _connectionIntervalMin;
        j["conn_interval_max"] = _connectionIntervalMax;
        j["repo_ms"] = _mouseReportIntervalMs;
    }

    void fromJson(JsonVariantConst j)
    {
        _version = j["version"];
        _chordScanTimeoutMs = j["chord_timeout"].as<uint32_t>();
        _deepSleepTimeoutMs = j["deepsleep_timeout"].as<uint32_t>();
        _lightSleepTimeoutMs = j["lightsleep_timeout"].as<uint32_t>();
        _joystickXDeadband = j["joy_x_deadband"].as<uint8_t>();
        _joystickYDeadband = j["joy_y_deadband"].as<uint8_t>();
        _mouseNegativeGain = j["mouse_negative_gain"].as<uint8_t>();
        _mickeyScale = j["mickey_scale"].as<float>();
        _txPower = j["tx_power"].as<int8_t>();
        _connectionIntervalMin = j["conn_interval_min"].as<uint16_t>();
        _connectionIntervalMax = j["conn_interval_max"].as<uint16_t>();
        _mouseReportIntervalMs = j["repo_ms"].as<uint32_t>();
    }
};