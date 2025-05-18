#include <ble/ble_hid.h>

#include <utils/timeout.h>
#include <utils/debug.h>

using namespace ble;

//uint16_t ble_hid::connectionHandle = BLE_CONN_HANDLE_INVALID;
//BLEHidAdafruit ble_hid::blehid;

void ble_hid::init()
{
    // 
    blehid.begin();
}

bool ble_hid::connect(uint8_t peerId, uint32_t timeoutMs, const ConnectionParam params)
{
    // 接続済みなら終了
    if (isConnected()) {
        return true;
    }

    // 接続設定
    Bluefruit.Periph.setConnInterval(params.connectionIntervalMin, params.connectionIntervalMax);
    Bluefruit.Periph.setConnSlaveLatency(params.slaveLatency);
    Bluefruit.setTxPower(params.txPower);

    // アドレス設定
    auto currentAddr = Bluefruit.getAddr();
    auto generatedAddress = generateAddress(peerId);
    if (memcmp(&currentAddr, &generatedAddress, sizeof(ble_gap_addr_t)) != 0) {
        auto r = Bluefruit.setAddr(&generatedAddress);
        DEBUG_PRINTF("%s", r ? "OK": "NG");
    }

    // アドバタイズ設定
    Bluefruit.Periph.setConnectCallback([] (uint16_t c) {
        DEBUG_PRINTF("onConnectCallback");
        connectionHandle = c;
    });
    Bluefruit.Periph.setDisconnectCallback(nullptr);
    Bluefruit.Advertising.clearData();
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
    Bluefruit.Advertising.addName();
    Bluefruit.Advertising.restartOnDisconnect(false);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.addService(blehid);

    // アドバタイズ開始
    auto success = Bluefruit.Advertising.start(timeoutMs / 1000);
    if (!success) {
        DEBUG_PRINTF("start advertising failed!");
        return false;
    }

    // 接続待ち
    auto connected = waitCondition(isConnected, timeoutMs);
    Bluefruit.Advertising.stop();

#if 1
    if (isConnected()) {
        auto connection = Bluefruit.Connection(connectionHandle);
        auto s = connection->requestPHY(BLE_GAP_PHY_2MBPS);
        DEBUG_PRINTF("request ble phy %d", s);
        s = connection->requestMtuExchange(247);
        DEBUG_PRINTF("request ble mtu %d", s);
    }
#endif

    return connected;
}


void ble_hid::disconnect(const uint32_t timeoutMs)
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


bool ble_hid::isConnected()
{
    return Bluefruit.connected(connectionHandle);
}


void ble_hid::mouseMove(const int8_t x, const int8_t y)
{
    blehid.mouseMove(x, y);
}


void ble_hid::mouseHScroll(const int8_t move)
{
    blehid.mousePan(move);
}

void ble_hid::mouseVScroll(const int8_t move)
{
    blehid.mouseScroll(move);
}

void ble_hid::mousePress(const MouseButton button)
{
    blehid.mouseButtonPress(button);
}

void ble_hid::mouseRelease(const MouseButton button)
{
    blehid.mouseButtonRelease(button);
    blehid.mouseMove(0, 0); // なぜかマウス移動レポートを送らないとRELEASEイベントを送ってくれない
}

void ble_hid::keyPress(const uint8_t scancode, const uint8_t modifierFlag)
{
    uint8_t scancodes[6] = {scancode, 0, 0, 0, 0, 0};
    blehid.keyboardReport(modifierFlag, scancodes);
}

void ble_hid::keyRelease(const uint8_t modifierFlag)
{
    uint8_t scancodes[6] = {0, 0, 0, 0, 0, 0};
    blehid.keyboardReport(modifierFlag, scancodes);
}
