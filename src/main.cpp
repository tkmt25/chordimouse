#include <Arduino.h>

#include <utility>
#include <utils/debug.h>
#include <alias.h>
#include <ble/ble_hid.h>
#include <ble/ble_config.h>

#include <config/config_manager.h>
#include <utils/internal_fs.h>

enum Mode {
  CONFIG,
  DEVICE
};

static constexpr module::Color LAYER_COLORS[2] = {
  module::Color::BLUE,
  module::Color::GREEN
};

static click_detector joystick_button;
static keyboard_layer keyboardLayer;
static key_profile profile;
static mouse_layer mouseLayer;
static int currentLayer = 0;

/**
 * @brief 設定を反映
 */
void applyConfig()
{
  // config
  {
    auto& cfg = config_manager::getGlobalConfig();
    mouseLayer.configure(cfg.getMouseNegativeGain(), cfg.getMickeyScale(), cfg.getMouseReportIntervalMs());
  }

  // keyprof
  {
    keyboardLayer.setKeyProfiles(config_manager::getKeyProfiles());
  }

  // calibration
  {
    auto& calib = config_manager::getCalibration();
    mouseLayer.cariblate(calib);
    keyboardLayer.cariblate(calib);
  }
}

/**
 * @brief 初期化処理
 */
void setup()
{
  debugInit();
  stopwatch_ms();

#if 0
  // InternalFSをクリア
  {
    if (InternalFS.begin()) {
      InternalFS.format();
    }
  }
#endif

  // 外部Flashは使わないので停止
  {
    stopwatch_ms();
    sleep_controller::enterSleepQSPIFlash();
  }
  

  // ピン初期設定
  button1::assign();
  button2::assign();
  button3::assign();
  button4::assign();
  middle_button1::assign();
  mode_sw::assign();
  joystick::assign();
  led_indicator::assign();

  // 未使用ピン
  pinMode(gpio::UNUSED_1, OUTPUT); 
  digitalWrite(gpio::UNUSED_1, LOW);
  pinMode(gpio::UNUSED_2, OUTPUT); 
  digitalWrite(gpio::UNUSED_2, LOW);

  // 設定値読み込み
  config_manager::init();
  applyConfig();

  // BLE初期化
  ble::init();
  ble::ble_hid::init();
  ble::ble_config::init();

#if 0
  Bluefruit.Periph.clearBonds();
#endif

  // セットアップ済み
  led_indicator::turnOnWith(LAYER_COLORS[currentLayer]);

  // スリープカウンタリセット
  sleep_controller::resetSleepCount();
}


/**
 * @brief メインループ
 */
void loop()
{
  static Mode mode = Mode::DEVICE;
  bool wasAction = false;
  stopwatch_ms();

  // モードトグル
  joystick_button.update();
  if (joystick_button.isLongPressed()) {
    joystick_button.reset();

    switch(mode) {
      case DEVICE:
      {
        mode = Mode::CONFIG;
        led_indicator::startBlink(module::Color::PURPLE, 500);
        ble::ble_hid::disconnect();
        break;
      }
        
      case CONFIG:
      {
        mode = Mode::DEVICE;
        led_indicator::stopBlink(); 
        ble::ble_config::disconnect();
        led_indicator::turnOnWith(LAYER_COLORS[currentLayer]);
        break;
      }
    }
  }

  // メイン処理
  switch(mode) {

    // 設定モード
    case Mode::CONFIG:
    {
      {
        if (!ble::ble_config::isConnected()) {
          DEBUG_PRINTF("start advertising ble config");

          ble::ble_config::setUpdateConfigCallback([](const config& cfg) {
            DEBUG_PRINTF("config updated by ble %f", cfg.getMickeyScale());
            config_manager::saveConfig(cfg);
            applyConfig();
          });
          ble::ble_config::setUpdateKeyProfCallback([](const key_profiles& profs) {
            DEBUG_PRINTF("updated key profiles ");
            config_manager::saveKeyProfiles(profs);
            applyConfig();
          });
          ble::ble_config::setDisconnectCallback([](){
            mode = Mode::DEVICE;
          });

          ble::ble_config::connect(config_manager::getGlobalConfig(), config_manager::getKeyProfiles(), 5000);
        }
      }
    }
    break;


    // デバイスモード
    case Mode::DEVICE:
    {
      // BLEアドバタイズ(完了まで待機)
      // TODO: ボタン切り替え未実装のためとりあえず固定
      //ble::begin(mode_sw::isPressed() ? 0 : 1);
      if (!ble::ble_hid::isConnected()){
        stopwatch_ms("ble connect");
        auto& cfg = config_manager::getGlobalConfig();
        led_indicator::startBlink(module::Color::BLUE, 500);
        bool connected = ble::ble_hid::connect(1, 5000, ble::ConnectionParam{
          .connectionIntervalMin = cfg.getConnectionIntervalMin(),
          .connectionIntervalMax = cfg.getConnectionIntervalMax(),
          .slaveLatency = 0,
          .timeout = 2000,
          .txPower = cfg.getTxPower()
        });

        if (connected) {
          DEBUG_PRINTF("connected hid!");
          led_indicator::stopBlink();
          led_indicator::turnOnWith(LAYER_COLORS[currentLayer]);

          {
            mouseLayer.configure(cfg.getMouseNegativeGain(), cfg.getMickeyScale(), cfg.getMouseReportIntervalMs());
          }

          wasAction = true;
        }
        
        goto FINALIZE;
      }

      // 入力処理
      switch (currentLayer)
      {
        case 0: // mouse layer
        {
          if (joystick_button.isClicked()) {
            currentLayer = 1;
            led_indicator::turnOnWith(LAYER_COLORS[currentLayer]);
            break;
          }
          wasAction = mouseLayer.action();
          break;
        }

        case 1: // keyboard layer
        {
          if (joystick_button.isClicked()) {
            currentLayer = 0;
            led_indicator::turnOnWith(LAYER_COLORS[currentLayer]);

            // 全てボタン離す
            // TODO: 無理やりやってるので後でコード整理
            ble::ble_hid::keyRelease();
            applyConfig(); // TODO: リセット処理は別にしたい

            break;
          }
          auto [chord, event] = keyboardLayer.scanChord(config_manager::getGlobalConfig().getChordScanTimeoutMs());
          wasAction = keyboardLayer.action(chord, event);
          break;
        }
      }
    }

    break;
  }

FINALIZE:

  // 操作ありならスリープカウントリセット
  if (wasAction || mode==Mode::CONFIG)
  {
    sleep_controller::resetSleepCount();
  }

  // スリープ処理
  if (sleep_controller::shouldEnterSleep(config_manager::getGlobalConfig().getLightSleepTimeoutMs()))
  {
    led_indicator::turnOff();
    led_indicator::stopBlink();
    
    // SystemONSleepでタイムアウトした場合はDeepSleepに移行
    auto wasTimeout = sleep_controller::enterLightSleep(config_manager::getGlobalConfig().getDeepSleepTimeoutMs());
    if (wasTimeout) {
      sleep_controller::enterDeepSleep();
      // ここからは復帰しない→setup()に戻る
    }

    // lightsleep通常復帰
    led_indicator::turnOnWith(LAYER_COLORS[currentLayer]);
    applyConfig();
    sleep_controller::resetSleepCount();
  }
  //delayMicroseconds(500);
  //yield();
}
