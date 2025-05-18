#pragma once

#include <utils/debug.h>

#include <stdint.h>
#include <utility>
#include <config/key_profile.h>
#include <layer/event.h>
#include <modules/button.h>
#include <modules/joystick.h>

#include <config/calibration.h>
#include <utils/edge_detector.h>
#include <utils/axis_detector.h>

#include <ble/ble_hid.h>

namespace layer
{

  template <typename button1, typename button2, typename button3, typename button4, typename middle_button1, typename joystick>
  class keyboard_layer
  {
  public:
    keyboard_layer() = default;

    /**
     * @brief センサーをキャリブレーションする
     * 
     */
    void inline cariblate(const calibration& calib)
    {
      _joystick_x.cariblate(calib.centerX);
      _joystick_y.cariblate(calib.centerY);
    }


    /**
     * @brief  Chord入力をスキャンする
     * @param timeoutMs スキャンのタイムアウト時間
     * @return [入力ソース,イベント].発生イベントを全て組み合わせた値を返す
     * @note ワンショットだとChord入力(同時押し)を検知できないことがあるので、タイムアウトまで繰り返しスキャンする
     */
    std::pair<uint16_t, uint16_t> inline scanChord(uint32_t timeoutMs)
    {
      auto start = millis();

      uint16_t chord = 0;
      uint16_t event = 0;

      _button1.update();
      _button2.update();
      _button3.update();
      _button4.update();
      _middle_button1.update();
      

      // タイムアウトするまで繰り返しスキャンする
      while (millis() - start < timeoutMs)
      {

        // ボタンリリースされたらchord無効で即時返却
        if (_button1.isFalling() || _button2.isFalling() || _button3.isFalling() || _button4.isFalling() || _middle_button1.isFalling())
        {
          return {chord, Event::RELEASE};
        }

        // chord判定
        if (_button1.isPressed())
        {
          chord |= Input::BUTTON_1;
          event = Event::PRESS;
        }
        if (_button2.isPressed())
        {
          chord |= Input::BUTTON_2;
          event = Event::PRESS;
        }
        if (_button3.isPressed())
        {
          chord |= Input::BUTTON_3;
          event = Event::PRESS;
        }
        if (_button4.isPressed())
        {
          chord |= Input::BUTTON_4;
          event = Event::PRESS;
        }
        if (_middle_button1.isPressed())
        {
          chord |= Input::MIDDLE_BUTTON_1;
          event = Event::PRESS;
        }

        // ボタン更新
        _button1.update();
        _button2.update();
        _button3.update();
        _button4.update();
        _middle_button1.update();
      }

      DEBUG_PRINTF("chord: 0x%04x, event: 0x%04x", chord, event);
      return {chord, event};
    }


    /**
     * @brief Chord入力に対するアクションを実行する
     * @param chord  Chord入力
     * @param event イベント
     *
     */
    bool inline action(const uint16_t chord, const uint16_t event)
    {
      bool wasAction = false;
      auto modifier = scanModifier();
      int layer = _joystick_x.isUp() ? 1 : 0; // FnキーONなら1
      auto& profile = _profiles[layer];
      
      if (event == Event::RELEASE)
      {
        ble::ble_hid::keyRelease(modifier);
        wasAction = true;
        goto FINALIZE;
      }

      if (profile.exists(chord))
      {
        ble::ble_hid::keyPress(profile.getScancode(chord), modifier);
        wasAction = true;
      }
      else if(_previous_modifier != modifier)
      {
        ble::ble_hid::keyPress(0, modifier);
        wasAction = true;
      }

      FINALIZE:
      _previous_modifier = modifier;
      return wasAction;
    }


    void inline setKeyProfiles(const key_profiles& profiles)
    {
      _profiles = profiles;
    }


  private:
    key_profiles _profiles;
    edge_detector<button1> _button1;
    edge_detector<button2> _button2;
    edge_detector<button3> _button3;
    edge_detector<button4> _button4;
    edge_detector<middle_button1> _middle_button1;
    edge_detector<typename joystick::pushButton> _joystick_button;

    axis_detector<typename joystick::xAxis> _joystick_x;
    axis_detector<typename joystick::yAxis> _joystick_y;

    uint8_t _previous_modifier = 0;

    /**
     * @brief Modifierキーをスキャンする
     * 
     * @return 押下されたModifierキー(ビットフラグ)
     */
    uint8_t scanModifier()
    {
      uint8_t pressedModifier = ble::ble_hid::Modifier::NONE;

      _joystick_y.update();
      _joystick_x.update();

      /*
       *   JU
       * JL  JR
       *   JD
       * JU = y Down
       * JD = y Up
       * JL = x Down
       * JR = x Up
       */
      if (_joystick_y.isDown()) pressedModifier |= ble::ble_hid::Modifier::LEFT_CTRL;  // JU
      if (_joystick_y.isUp())   pressedModifier |= ble::ble_hid::Modifier::LEFT_ALT;   // JD
      if (_joystick_x.isDown()) pressedModifier |= ble::ble_hid::Modifier::LEFT_SHIFT; // JL

      return pressedModifier;
    }
  };

}