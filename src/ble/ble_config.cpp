#include <ble/ble_config.h>
#include <utils/timeout.h>
#include <utils/debug.h>

using namespace ble;

void ble_config::init()
{
    configService.begin();

    // グローバル設定Characteristic
    {
        globalConfigChar.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
        globalConfigChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
        globalConfigChar.setMaxLen(config::MAX_BUFFSIZE);
        globalConfigChar.setWriteCallback([](uint16_t conn_handle, BLECharacteristic *chr, uint8_t *data, uint16_t len) {
            config cfg;
            auto success = cfg.deserialize(data, len);
            if (!success) {
                // デシリアライズできない=JSON不正なら更新・通知せず終了
                return ;
            }
            
            // バッファ更新
            chr->write(data, len);

            // コールバック
            if (updateConfigCallback) {
                updateConfigCallback(cfg);
            } 
        });
        globalConfigChar.begin();
    }

    // キープロファイル設定Characteristic
    {
        keyProfileConfigChar.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
        keyProfileConfigChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
        keyProfileConfigChar.setMaxLen(key_profiles::MAX_SIZE);
        keyProfileConfigChar.setWriteCallback([](uint16_t conn_handle, BLECharacteristic *chr, uint8_t *data, uint16_t len){
            key_profiles profs;
            auto success = profs.deserialize(data, len);
            if (!success) {
                return ;
            }

            chr->write(data, len);

            if (updateKeyprofCallback) {
                updateKeyprofCallback(profs);
            }
        });
        keyProfileConfigChar.begin();
    }
}


bool ble_config::connect(const config& cfg, const key_profiles& profs, const uint32_t timeoutMs)
{
    if (isConnected()) {
        return true;
    }

    connectionHandle = BLE_CONN_HANDLE_INVALID;

    auto currentAddr = Bluefruit.getAddr();
    auto generatedAddress = generateAddress(3);
    if (memcmp(&currentAddr, &generatedAddress, sizeof(ble_gap_addr_t)) != 0) {
        auto r = Bluefruit.setAddr(&generatedAddress);
        DEBUG_PRINTF("%s", r ? "OK": "NG");
    }

    Bluefruit.Periph.setConnectCallback([](uint16_t c){
        connectionHandle = c;
    });
    Bluefruit.Periph.setDisconnectCallback([](uint16_t c, uint8_t r) {
        connectionHandle = BLE_CONN_HANDLE_INVALID;
        if(disconnectCallback) {
            disconnectCallback();
        }
    });

    // バッファ更新
    {
        uint8_t buff[config::MAX_BUFFSIZE];
        auto size = cfg.serialize(buff);
        globalConfigChar.write(buff, size);
    }
    {
        uint8_t buff[key_profiles::MAX_SIZE];
        auto size = profs.serialize(buff);
        keyProfileConfigChar.write(buff, size);
    }

    // アドバタイズ設定
    Bluefruit.Advertising.clearData();
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.ScanResponse.clearData();
    Bluefruit.ScanResponse.addTxPower();
    Bluefruit.ScanResponse.addName();
    Bluefruit.ScanResponse.addService(configService);
    Bluefruit.Advertising.restartOnDisconnect(false);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms

    // アドバタイズ開始
    auto success = Bluefruit.Advertising.start(timeoutMs / 1000); 
    if (!success) {
        DEBUG_PRINTF("failed to start advertising!");
        return false;
    }

    // 接続待機
    auto connected = waitCondition(isConnected, timeoutMs);
    Bluefruit.Advertising.stop();
    return connected;
}


bool ble_config::isConnected()
{
    return Bluefruit.connected(connectionHandle);
}


void ble_config::disconnect(const uint32_t timeoutMs)
{
    auto connection = Bluefruit.Connection(connectionHandle);

    // 未接続なら終了
    if (!connection || (connection && !connection->connected())) {
        connectionHandle = BLE_CONN_HANDLE_INVALID;
        return ;
    }

    connection->disconnect();

    // 切断待機
    auto disconnected = waitCondition([](){
        return !isConnected();
    }, timeoutMs);

    if (disconnected) {
        connectionHandle = BLE_CONN_HANDLE_INVALID;
    }
}