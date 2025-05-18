#pragma once

#include <Arduino.h>
#include <nrf_lpcomp.h>
#include <nrf_sdm.h>
#include <nrf_power.h>
#include <nrf_gpio.h>
#include <nrf_nvic.h>
#include <ble.h>
#include <ble/ble_hid.h>
#include <ble/ble_config.h>

#include <bluefruit.h>
#include <utils/debug.h>
#include <Adafruit_SPIFlash.h>

#define ENABLE_LPCOMP_IRQ (1)
#define ENABLE_BLE_SETTING (0)

namespace utils 
{
    /**
     * @brief スリープ管理用クラス
     * @note SoftDeviceが有効な状態を前提としてます
     */
    template <typename button1, typename button2, typename button3, typename button4, typename middle_button1, typename joystick>
    class sleep_controller
    {
    public:
        sleep_controller() = delete;
    
        /**
         * @brief スリープを開始するかを判定する
         * @param timeoutMs スリープを開始するまでの時間
         * @return true: スリープを開始する
         */
        static bool shouldEnterSleep(uint32_t timeoutMs)
        {
            if (_startMs == 0)
            {
                _startMs = millis();
                return false;
            }
    
            if (millis() - _startMs > timeoutMs)
            {
                return true;
            }
    
            return false;
        }


        /**
         * @brief スリープカウンタをリセットする
         */
        static void resetSleepCount()
        {
            _startMs = millis();
        }


        /**
         * @brief SystemONSleep(Constant)に移行する。
         * 
         * @param timeoutMs [in] スリープタイムアウト時間(ms)。デフォルトはタイムアウトなし
         * 
         * @retval true  タイムアウトした
         * @retval false タイムアウトせずGPIO/LPCOMP割込みで復帰した
         */
        static bool enterLightSleep(const uint32_t timeoutMs=0)
        {
            stopwatch_ms();
            DEBUG_PRINTF(">> light sleep start!");

            prepareInterruptForSleep();

#if ENABLE_BLE_SETTING            
            // ble::disconnect(true);
            //ble::update(IDLE_CONNECTION_PARAMETER);
#endif

            // タイムアウト用
            if (timeoutMs > 0) {
                enableRtcIntrrupt(timeoutMs);
            }
    
            // WDT停止
            NRF_WDT->CONFIG |= (WDT_CONFIG_SLEEP_Pause << WDT_CONFIG_SLEEP_Pos);

            // スリープ開始
            sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
            sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);

            {
                stopwatch_ms("light sleep");
                
                // スリープ本体
                while(!(NRF_RTC2->EVENTS_COMPARE[0] ||  // RTC2
                    _lpcompIntFired || // LPCOMP(Joystick)
                    NRF_P0->LATCH || NRF_P1->LATCH) // GPIO(Button))) 
                ){
                    sd_app_evt_wait();
#if !ENABLE_LPCOMP_IRQ
                    if (NRF_LPCOMP->EVENTS_CROSS) {
                        _lpcompIntFired = true;
                    }
#endif

                }
                // スリープここまで
            }

            DEBUG_PRINTF("<< light sleep wakeup");
            bool timeouted = NRF_RTC2->EVENTS_COMPARE[0];

            // 割り込み解除
            disableGpioIntrrupt();
            disableLpcompIntrrupt();
            disableRtcIntrrupt();

            // BLE接続設定を元に戻す
#if ENABLE_BLE_SETTING
            if( !ble::update(ble::ACTIVE_CONNECTION_PARAMETER) ) {
                // PC側で30sぐらい変更ガードが働いてるため、スリープが30s以内に解除されると失敗することがある（おそらくconnectionIntervalを縮めた場合）
                // 失敗してるならもう切断してmain側の再接続に任せる
                ble::disconnect(true);
            }
#endif            

            return timeouted;
        }

        
        /**
         * @brief DeepSleepを開始する
         * @note 処理は戻らないことに注意
         */
        static void enterDeepSleep()
        {
            DEBUG_PRINTF(">>> deep sleep start ! ");
    
            prepareInterruptForSleep();

            // BLE切断
            ble::ble_hid::disconnect();
            ble::ble_config::disconnect();

            // softdevice終了
            disableSoftDevice();

            DEBUG_PRINTF("<<< System OFF Sleep! (not return)");

            nrf_power_system_off(NRF_POWER);
            // wakeup by GPIO/LPCOMP
        }


        /**
         * XIAOのオンボードQSPI Flashをスリープする
         */
        static void enterSleepQSPIFlash() {
            Adafruit_FlashTransport_QSPI flashTransport;
            Adafruit_SPIFlash flash(&flashTransport);

            flashTransport.begin();
            flashTransport.runCommand(0xB9);
            delayMicroseconds(5);
            flashTransport.end();
        }

        static volatile bool _lpcompIntFired;


    private:
        static uint32_t _startMs;

#if ENABLE_BLE_SETTING                    
        static constexpr ble::ble_hid::ConnectionParameter IDLE_CONNECTION_PARAMETER = {
            ble::ACTIVE_CONNECTION_PARAMETER.connectionInterval,  // connectionIntervalは変更するとセントラルから拒否られるので変えない
            79,   // slaveLatency 
            3200, // sup timeout 30000ms
            -4,   // txpower -4dbm
        };
#endif
        

        /**
         * @brief  割り込みを準備します
         */
        static inline void prepareInterruptForSleep()
        {
            // ボタン割り込み設定
            enableGpioIntrrupt();

            // LPCOMP設定
            enableLpcompIntrrupt();
        }


        /**
         * @brief SoftDeviceを停止し完了まで待機する
         */
        static inline void disableSoftDevice()
        {
            if (IsEnabledSoftDevice()) {
                sd_softdevice_disable();

                while(IsEnabledSoftDevice()) {
                    DEBUG_PRINTF("waiting softdevice disable...");
                    delay(1);
                }
            }
        }

        /**
         * @brief SoftDeviceが有効か
         * @return SoftDeviceが有効ならtrue
         */
        static inline bool IsEnabledSoftDevice()
        {
            uint8_t softdeviceEnabled;
            sd_softdevice_is_enabled(&softdeviceEnabled);
            return softdeviceEnabled;
        }


        /**
         * @brief RTC2割り込みを有効化
         * 
         * @param [in] timeoutMs 割り込み発生までのタイムアウト(ms)
         */
        static inline void enableRtcIntrrupt(const uint32_t timeoutMs)
        {
            NRF_RTC2->TASKS_STOP = 1;
            NRF_RTC2->TASKS_CLEAR = 1;

            NRF_RTC2->PRESCALER = 32767; // 1ms/tick
            NRF_RTC2->CC[0] = timeoutMs;
            NRF_RTC2->EVENTS_COMPARE[0] = 0;

            NRF_RTC2->INTENSET |= RTC_INTENSET_COMPARE0_Msk;
            NRF_RTC2->TASKS_START = 1;
        }


        /**
         * @brief RTC2割り込みを解除
         */
        static inline void disableRtcIntrrupt()
        {
            NRF_RTC2->EVENTS_COMPARE[0] = 0;
            NRF_RTC2->INTENCLR = RTC_INTENCLR_COMPARE0_Msk;
            NRF_RTC2->TASKS_STOP = 1;
        }


        /*
         * @brief X/Y軸のLPCOMP割り込みを有効化
         * @details ジョイスティックX軸/Y軸の電圧をLPCOMPで比較し、軸が動いたら割込みをかける(何かしら動いたらX/Y軸の電圧が対象じゃなくなるはず。一応ヒステリシスを入れる)
         */
        static inline void enableLpcompIntrrupt()
        {
            // 外部ピンを基準電圧にする
            NRF_LPCOMP->REFSEL = LPCOMP_REFSEL_REFSEL_ARef;
    
            // 基準電圧(X軸)
            NRF_LPCOMP->EXTREFSEL = joystick::xAxis::getPin() << LPCOMP_EXTREFSEL_EXTREFSEL_Pos;
    
            // 入力(Y軸)
            NRF_LPCOMP->PSEL = joystick::yAxis::getPin() << LPCOMP_PSEL_PSEL_Pos;
    
            // 入力モードCROSS(上下検知)
            NRF_LPCOMP->ANADETECT = LPCOMP_ANADETECT_ANADETECT_Cross << LPCOMP_ANADETECT_ANADETECT_Pos;
    
            // ヒステリシス(50mV)有効
            NRF_LPCOMP->HYST = LPCOMP_HYST_HYST_Hyst50mV << LPCOMP_HYST_HYST_Pos;
    
            // 割込み設定
            NRF_LPCOMP->INTENSET = LPCOMP_INTENSET_CROSS_Msk;

    
            // LPCOMP開始
            NRF_LPCOMP->EVENTS_READY = 0;
            NRF_LPCOMP->ENABLE = LPCOMP_ENABLE_ENABLE_Enabled;
            NRF_LPCOMP->TASKS_START = 1;

            // 開始待機
            while (NRF_LPCOMP->EVENTS_READY == 0);
            NRF_LPCOMP->EVENTS_READY = 0;
            NRF_LPCOMP->EVENTS_CROSS = 0;

            // 割込みハンドラ
#if ENABLE_LPCOMP_IRQ            
            sd_nvic_ClearPendingIRQ(LPCOMP_IRQn);
            sd_nvic_SetPriority(LPCOMP_IRQn, 3);
            sd_nvic_EnableIRQ(LPCOMP_IRQn);
#endif
            _lpcompIntFired = false;
        }


        /**
         * @brief LPCOMP割り込みを停止
         */
        static inline void disableLpcompIntrrupt()
        {
            // 割り込みハンドラ解除
#if ENABLE_LPCOMP_IRQ            
            sd_nvic_DisableIRQ(LPCOMP_IRQn);
#endif

            // LPCOMP割り込み解除
            NRF_LPCOMP->TASKS_STOP = 1;
            NRF_LPCOMP->INTENCLR = 0xFFFFFFFF;
            NRF_LPCOMP->ENABLE = LPCOMP_ENABLE_ENABLE_Disabled;


            // LPCOMPイベントクリア
            NRF_LPCOMP->EVENTS_CROSS = 0;
            NRF_LPCOMP->EVENTS_READY = 0;
            _lpcompIntFired = false;
        }
    

        /*
         * @brief GPIO(ボタン)割り込みを開始
         */
        static inline void enableGpioIntrrupt()
        {
            nrf_gpio_cfg_sense_input(g_ADigitalPinMap[button1::getPin()], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
            nrf_gpio_cfg_sense_input(g_ADigitalPinMap[button2::getPin()], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
            nrf_gpio_cfg_sense_input(g_ADigitalPinMap[button3::getPin()], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
            nrf_gpio_cfg_sense_input(g_ADigitalPinMap[button4::getPin()], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
            nrf_gpio_cfg_sense_input(g_ADigitalPinMap[middle_button1::getPin()], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);

            NRF_P0->LATCH = NRF_P0->LATCH;
            NRF_P1->LATCH = NRF_P1->LATCH;
        }

        /*
         * @brief GPIO(ボタン)割り込みを停止
         */
        static inline void disableGpioIntrrupt()
        {
            // GPIOイベントクリア
            NRF_P0->LATCH = NRF_P0->LATCH;
            NRF_P1->LATCH = NRF_P1->LATCH;

            // GPIO割り込み解除
            nrf_gpio_cfg_sense_set(g_ADigitalPinMap[button1::getPin()], NRF_GPIO_PIN_NOSENSE);
            nrf_gpio_cfg_sense_set(g_ADigitalPinMap[button2::getPin()], NRF_GPIO_PIN_NOSENSE);
            nrf_gpio_cfg_sense_set(g_ADigitalPinMap[button3::getPin()], NRF_GPIO_PIN_NOSENSE);
            nrf_gpio_cfg_sense_set(g_ADigitalPinMap[button4::getPin()], NRF_GPIO_PIN_NOSENSE);
            nrf_gpio_cfg_sense_set(g_ADigitalPinMap[middle_button1::getPin()], NRF_GPIO_PIN_NOSENSE);
        }

    };
}
