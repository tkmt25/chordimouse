#pragma once

#include <utils/debug.h>
#include <stdint.h>
#include <modules/button.h>
#include <modules/joystick.h>

#include <config/calibration.h>
#include <utils/edge_detector.h>
#include <utils/axis_detector.h>
#include <utils/cursor_strategy.h>

#include <ble/ble_hid.h>

namespace layer
{

    /**
     * @brief マウス制御をおこなうクラス
     * @tparam button1 
     * @tparam button2 
     * @tparam button3 
     * @tparam button4
     * @tparam middle_button1 
     * @tparam joystick
     */
    template <typename button1, typename button2, typename button3, typename button4, typename middle_button1, typename joystick>
    class mouse_layer
    {
    public:
        mouse_layer(){}

        /**
         * @brief キャリブレーションする
         */
        void inline cariblate(const calibration& calib)
        {
            _joystick_x.cariblate(calib.centerX);
            _joystick_y.cariblate(calib.centerY);
        }

        void inline configure(const uint8_t gain, const float scale, const uint32_t reportIntervalMs)
        {
            auto& strategy = _sampler.getStorategy();
            strategy.setGain(gain);
            strategy.setMickeyScale(scale);
            _sampler.setInterval(reportIntervalMs);
            _sampler.reset();
        }


        /**
         * @brief ボタン/ジョイスティックをスキャンしBLE HIDマウスレポートを送信
         * @return イベントが発生したか
         */
        bool inline action()
        {
            bool wasAction = false;
            stopwatch_ms();

            // ホイール判定
            _middle_button1.update();
            _joystick_x.update();
            _joystick_y.update();
            
            //debugPrintf("x:%04d, y:%04d, ", _joystick_x.getValue(), _joystick_y.getValue());

            // マウス/ホイール移動
            {
                auto x = _joystick_x.getMove();
                auto y = _joystick_y.getMove();
                auto [moveX, moveY] = _sampler.getMoveCursor(x, y);

                DEBUG_PRINTF("moveX: %d, moveY: %d", moveX, moveY);

                static uint32_t lastSendScrollMs = 0;

                // ホイール移動
                // 500msに一回送信するようにしてみる
                if (_middle_button1.isPressed())
                {
                    // TODO: 効きすぎなので暫定で抑えてみる
                    // 送信レートが100hzだから動きすぎるように感じるのかも？
                    // レート下げる工夫が必要かもしれない・・
                    if ((millis() - lastSendScrollMs) >= 150) {
                        #if 0
                        int8_t scrollX = (int8_t)map(x, -512, 512, 5, -5);
                        int8_t scrollY = (int8_t)map(y, -512, 512, 5, -5);
                        if (_joystick_x.isMoving()) ble::ble_hid::mouseHScroll(scrollX);
                        if (_joystick_y.isMoving()) ble::ble_hid::mouseVScroll(scrollY);
                        #endif
                        //if (_joystick_x.isMoving()) ble::ble_hid::mouseHScroll(-moveX);
                        if (abs(moveY) > 0) {
                            ble::ble_hid::mouseVScroll(-moveY);
                            lastSendScrollMs = millis();
                        }
                    }
                    
                    return true;
                }
                // マウス移動
                else if(moveX != 0 || moveY != 0) {
                    ble::ble_hid::mouseMove(moveX, moveY);
                    wasAction = true; 
                }
            }

            // 左クリック
            _button1.update();
            if (_button1.isRising())  { ble::ble_hid::mousePress(ble::ble_hid::MouseButton::LEFT); wasAction = true; }
            if (_button1.isFalling()) { ble::ble_hid::mouseRelease(ble::ble_hid::MouseButton::LEFT); wasAction = true; }

            // 右クリック
            _button2.update();
            if (_button2.isRising())  { ble::ble_hid::mousePress(ble::ble_hid::MouseButton::RIGHT); wasAction = true; }
            if (_button2.isFalling()) { ble::ble_hid::mouseRelease(ble::ble_hid::MouseButton::RIGHT); wasAction = true; }

            // 戻る
            _button3.update();
            if (_button3.isRising())  { ble::ble_hid::mousePress(ble::ble_hid::MouseButton::BACKWARD); wasAction = true; }
            if (_button3.isFalling()) { ble::ble_hid::mouseRelease(ble::ble_hid::MouseButton::BACKWARD); wasAction = true; }

            // 進む
            _button4.update();
            if (_button4.isRising())  { ble::ble_hid::mousePress(ble::ble_hid::MouseButton::FORWARD); wasAction = true; }
            if (_button4.isFalling()) { ble::ble_hid::mouseRelease(ble::ble_hid::MouseButton::FORWARD); wasAction = true; }

            return wasAction;
        }

    private:
        edge_detector<button1> _button1;
        edge_detector<button2> _button2;
        edge_detector<button3> _button3;
        edge_detector<button4> _button4;
        edge_detector<middle_button1> _middle_button1;
        edge_detector<typename joystick::pushButton> _joystick_button;

        axis_detector<typename joystick::xAxis> _joystick_x;
        axis_detector<typename joystick::yAxis> _joystick_y;
        //linear_strategy _move_strategy;
        //negative_inertia_strategy _move_strategy;
        sampler<negative_inertia_strategy> _sampler{10};
    };

}