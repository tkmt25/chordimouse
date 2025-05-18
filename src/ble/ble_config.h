#pragma once

#include <bluefruit.h>
#include <config/config.h>
#include <config/key_profile.h>
#include <ble/ble_common.h>

namespace ble
{
    /**
     * @brief 設定モードを表すクラス
     */
    class ble_config
    {
    public:
        using UpdateConfigCallback = void(*)(const config& cfg); ///< 設定更新コールバック
        using UpdateKeyprofCallback = void(*)(const key_profiles& keyProfs); ///< キープロファイル更新コールバック
        using DisconnectCallback = void(*)(); ///< 切断コールバック

        ble_config() = delete;

        static void init();
        static bool connect(const config& cfg, const key_profiles& profs, const uint32_t timeoutMs=0);
        static bool isConnected();
        static void disconnect(const uint32_t timeoutMs=0);
        static void setUpdateConfigCallback(UpdateConfigCallback callback)
        {
            updateConfigCallback = callback;
        }
        static void setUpdateKeyProfCallback(UpdateKeyprofCallback callback)
        {
            updateKeyprofCallback = callback;
        }
        static void setDisconnectCallback(DisconnectCallback callback)
        {
            disconnectCallback = callback;
        }


    private:
        static constexpr auto CONFIG_SERVICE_UUID = 0xF00D;
        static constexpr auto CONFIG_CHR_GLOBAL_UUID = 0xFF01;
        static constexpr auto CONFIG_CHR_KEYPROF_UUID = 0xFF02;

        static inline BLEService configService{CONFIG_SERVICE_UUID};
        static inline BLECharacteristic globalConfigChar{CONFIG_CHR_GLOBAL_UUID};
        static inline BLECharacteristic keyProfileConfigChar{CONFIG_CHR_KEYPROF_UUID};
        static inline UpdateConfigCallback updateConfigCallback = nullptr;
        static inline UpdateKeyprofCallback updateKeyprofCallback = nullptr;
        static inline DisconnectCallback disconnectCallback = nullptr;
        static inline uint16_t connectionHandle = BLE_CONN_HANDLE_INVALID;
    };

};