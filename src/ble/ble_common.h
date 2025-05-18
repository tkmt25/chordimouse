#pragma once

#include <bluefruit.h>

namespace ble
{
    /**
     * @brief 接続用パラメータ
     */
    struct ConnectionParam
    {
        uint16_t connectionIntervalMin = 6;
        uint16_t connectionIntervalMax = 9;
        uint16_t slaveLatency = 0;
        uint16_t timeout = 2000;
        int8_t txPower = 4;
    };


    /**
     * @brief BLE初期化処理
     */
    inline void init()
    {
        static bool initialized = false;
        static BLEDis bledis;

        if (initialized)
        {
            return;
        }

        
        // 高速化
        //Bluefruit.configPrphBandwidth(BANDWIDTH_HIGH);
        Bluefruit.configPrphConn(128, 6, 6, BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT);

        // 初期化処理
        Bluefruit.begin(2); //
        Bluefruit.setName("ChordiMouse");

        Bluefruit.autoConnLed(false);
        
        // デバイス名を設定
        bledis.setManufacturer("tkmt25");
        bledis.setModel("ChordiMouse");
        bledis.begin();

        initialized = true;
    }


    /**
     * @brief BLEアドレスを生成する
     * @param [in] seed 種
     * @return 生成されたアドレス
     */
    inline ble_gap_addr_t generateAddress(const uint8_t seed)
    {
        // MACアドレスを生成・設定する
        // Bluefruitが自動保存するbondingはMACアドレスごとになっているため、MACアドレスを変えることで接続先を切り替える
        auto gap_addr = Bluefruit.getAddr();
        gap_addr.addr[0] = seed;
        return gap_addr;
    }
}