#pragma once

#include <math.h>


/**
 * @brief ジョイスティック軸の状態を取得するユーティリティクラス
 */
template <typename axis>
class axis_detector
{
    public:
        /**
         * @brief コンストラクタ
         * @param [in] deadHand 不感帯
         */
        axis_detector(const uint32_t deadHand = 5): _deadHand(deadHand) {}


        /**
         * @brief キャリブレーションする
         * @param [in] calibrated キャリブレーション済みデータ。未指定時は現在地で校正。
         */
        void inline cariblate(const uint32_t calibrated=0)
        {
            // キャリブレーション済みデータを読み込み
            _center = (calibrated == 0) ? axis::getValue() : calibrated;

            _previousValue = _center;
            _value = _center;
        }


        /**
         * @brief 状態を更新
         */
        void inline update()
        {
            _previousValue = _value;
            _value = axis::getValue();

            _wasUp = _isUp;
            _wasDown = _isDown;

            _isUp = _value > (_center + THRE);
            _isDown = _value < (_center - THRE);
        }


        /**
         * @brief 中心からの差を取得
         * @return 中心からの差(-512～+512)
         */
        int32_t getMove()
        {
            return _value - _center;
        }


        /**
         * @brief センサ生データを取得
         * @return 0 - 1024
         */
        uint32_t inline getValue()
        {
            return _value;
        }

        /**
         * @brief 軸が動いているか
         */
        bool inline isMoving()
        {
            return (uint32_t)(abs(getMove())) > _deadHand;
        }


        /**
         * @brief 軸が上に傾いているか
         */
        bool inline isUp()
        {
            return _isUp;
        }


        /**
         * @brief 軸が下に傾いているか
         */
        bool inline isDown()
        {
            return _isDown;
        }


        /**
         * @brief 軸が中央か
         */
        bool inline isCenter()
        {
            return !_isUp && !_isDown;
        }


        /**
         * @brief 軸が上に立ち上がったか
         */
        bool inline isRisingUp()
        {
            return !_wasUp & _isUp;
        }


        /**
         * @brief 軸が上に立ち下がったか
         */
        bool inline isFallingUp()
        {
            return _wasUp & !_isUp;
        }


        /**
         * @brief 軸が下に立ち上がったか
         */
        bool inline isRisingDown()
        {
            return !_wasDown & _isDown;
        }


        /**
         * @brief 軸が下に立ち下がったか
         */
        bool inline isFallingDown()
        {
            return _wasDown & !_isDown;
        }
        
        
    private:
        uint32_t _center = 0;
        uint32_t _value = 0;
        uint32_t _deadHand = 0; ///< 不感帯
        uint32_t _previousValue = 0;

        bool _isUp = false;
        bool _isDown = false;
        bool _wasUp = false;
        bool _wasDown = false;

        static constexpr uint32_t MAX_VALUE = 1023;
        static constexpr uint32_t MIN_VALUE = 0;
        static constexpr uint32_t THRE = (MAX_VALUE - MIN_VALUE) / 4;
};

