#pragma once

#include <Arduino.h>


namespace module
{
    /**
     * @brief 色を表す列挙値
     */
    enum Color
    {
        RED = 0x04,
        GREEN = 0x02,
        BLUE = 0x01,
        PURPLE = RED | BLUE,
        YELLOW = RED | GREEN,
        CYAN = GREEN | BLUE,
        WHITE = RED | GREEN | BLUE,
    };


    /**
     * @brief LEDインジケータの制御クラス。アノードコモン
     * @tparam pinRed   赤のピン番号
     * @tparam pinGreen 緑のピン番号
     * @tparam pinBlue  青のピン番号
     */
    template <uint8_t pinRed, uint8_t pinGreen, uint8_t pinBlue>
    class led_indicator
    {
    public:
        led_indicator() = default;


        /**
         * @brief ピンをアサイン
         */
        constexpr static void assign()
        {
            pinMode(pinRed, OUTPUT);
            pinMode(pinGreen, OUTPUT);
            pinMode(pinBlue, OUTPUT);
        }


        /**
         * @brief 色を設定する
         * @param [in] color 色
         * 
         * @note 指定した色で点灯させるにはturnOnをコール
         */
        static inline void setColor(const Color color)
        {
            _color = color;
        }


        /**
         * @brief 点灯します
         * @note setColorで設定した色になります
         */
        constexpr static void turnOn()
        {
            // アノードコモンのため、LOWで点灯
            digitalWrite(pinRed,   !((_color & RED)   >> 2));
            digitalWrite(pinGreen, !((_color & GREEN) >> 1));
            digitalWrite(pinBlue,  !((_color & BLUE)  ));
        }


        /**
         * @brief 指定した色で点灯します
         * @param [in] color 色
         */
        constexpr static void turnOnWith(const Color color)
        {
            turnOff();
            setColor(color);
            turnOn();
        }


        /**
         * @brief 消灯します
         */
        constexpr static void turnOff()
        {
            digitalWrite(pinRed, HIGH);
            digitalWrite(pinGreen, HIGH);
            digitalWrite(pinBlue, HIGH);
        }


        constexpr static void startBlink(const Color color, const uint32_t intervalMs)
        {
            turnOff();
            setColor(color);
            startBlink(intervalMs);
        }

        /**
         * @brief 点滅を開始します
         * @param interval 点滅間隔(ms)
         * @note 色はsetRGBで設定した色になります
         */
        static inline void startBlink(const uint32_t intervalMs)
        {
            if (_isBlinking) {
                return ;
            }

            // タイマーカウント値を計算（31.25kHz で interval_ms 毎に割り込み）
            uint32_t ticks = 31250 * intervalMs / 1000;

            NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
            
            // -----------------------------
            // GPIOTE 設定（色ごとに設定）
            // 各ピンを「トグルするタスク」に割り当てる
            // -----------------------------

            if (_color & RED)
            {
                NRF_GPIOTE->CONFIG[GPIOTE_RED_CH] =
                    (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
                    (g_ADigitalPinMap[pinRed] << GPIOTE_CONFIG_PSEL_Pos) |
                    (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
                    (GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos); // 初期値LOW
            }

            if (_color & GREEN)
            {
                NRF_GPIOTE->CONFIG[GPIOTE_GREEN_CH] =
                    (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
                    (g_ADigitalPinMap[pinGreen] << GPIOTE_CONFIG_PSEL_Pos) |
                    (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
                    (GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos);
            }

            if (_color & BLUE)
            {
                NRF_GPIOTE->CONFIG[GPIOTE_BLUE_CH] =
                    (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
                    (g_ADigitalPinMap[pinBlue] << GPIOTE_CONFIG_PSEL_Pos) |
                    (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
                    (GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos);
            }

            // -----------------------------
            // TIMER1 設定
            // -----------------------------

            NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos; // タイマーモード
            NRF_TIMER1->PRESCALER = 9;                                       // 16MHz / 2^9 = 31.25kHz
            NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
            NRF_TIMER1->CC[0] = ticks;                                                                   // 指定間隔のカウント値設定
            NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos; // 自動リセット
            NRF_TIMER1->TASKS_START = 1;                                                                 // タイマー開始

            // -----------------------------
            // PPI 設定：タイマーイベント → GPIOTE タスクを接続
            // -----------------------------

            if (_color & RED)
            {
                NRF_PPI->CH[PPI_RED_CH].EEP = (uint32_t)&NRF_TIMER1->EVENTS_COMPARE[0];        // イベント：タイマー0一致
                NRF_PPI->CH[PPI_RED_CH].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[GPIOTE_RED_CH]; // タスク：GPIOTEで出力トグル
                NRF_PPI->CHENSET = (1 << PPI_RED_CH);                                          // PPIチャネル有効化
            }

            if (_color & GREEN)
            {
                NRF_PPI->CH[PPI_GREEN_CH].EEP = (uint32_t)&NRF_TIMER1->EVENTS_COMPARE[0];
                NRF_PPI->CH[PPI_GREEN_CH].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[GPIOTE_GREEN_CH];
                NRF_PPI->CHENSET = (1 << PPI_GREEN_CH);
            }

            if (_color & BLUE)
            {
                NRF_PPI->CH[PPI_BLUE_CH].EEP = (uint32_t)&NRF_TIMER1->EVENTS_COMPARE[0];
                NRF_PPI->CH[PPI_BLUE_CH].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[GPIOTE_BLUE_CH];
                NRF_PPI->CHENSET = (1 << PPI_BLUE_CH);
            }

            _isBlinking = true;
        }

        
        /**
         * @brief 点滅を停止します
         */
        static inline void stopBlink()
        {
            if (!_isBlinking) {
                return ;
            }

            // タイマー停止＆リセット
            NRF_TIMER1->TASKS_STOP = 1;
            NRF_TIMER1->TASKS_CLEAR = 1;

            // GPIOTE設定解除 & LED OFF
            if (_color & RED)
            {
                NRF_PPI->CHENCLR = (1 << PPI_RED_CH);
                NRF_GPIOTE->CONFIG[GPIOTE_RED_CH] = 0;
                digitalWrite(pinRed, HIGH);
            }

            if (_color & GREEN)
            {
                NRF_PPI->CHENCLR = (1 << PPI_GREEN_CH);
                NRF_GPIOTE->CONFIG[GPIOTE_GREEN_CH] = 0;
                digitalWrite(pinGreen, HIGH);
            }

            if (_color & BLUE)
            {
                NRF_PPI->CHENCLR = (1 << PPI_BLUE_CH);
                NRF_GPIOTE->CONFIG[GPIOTE_BLUE_CH] = 0;
                digitalWrite(pinBlue, HIGH);
            }

            _isBlinking = false;
        }

    private:
        inline static Color _color  = Color::GREEN;
        inline static bool _isBlinking = false;

        constexpr static int GPIOTE_RED_CH = 0;
        constexpr static int GPIOTE_GREEN_CH = 1;
        constexpr static int GPIOTE_BLUE_CH = 2;

        constexpr static int PPI_RED_CH = 0;
        constexpr static int PPI_GREEN_CH = 1;
        constexpr static int PPI_BLUE_CH = 2;
    };
}
