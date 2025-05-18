#pragma once

/**
 * @brief ボタンの立ち上がり/立下りを検知するユーティリティクラス
 */
template <typename button>
class edge_detector
{
    public:

        /**
         * @brief ボタン状態を更新する
         */
        void inline update()
        {
            bool current = button::isPressed();
            _isRising = current && !_previous;
            _isFalling = !current && _previous;
            _previous = current;
        }


        /**
         * @brief 立ち上がり検知
         * @retval true  立ち上がり発生
         * @retval false 未発生
         */
        bool inline isRising()
        {
            return _isRising;
        }


        /**
         * @brief 立ち下がり検知
         * @retval true  立ち下がり発生
         * @retval false 未発生
         */
        bool inline isFalling()
        {
            return _isFalling;
        }


        /**
         * @brief ボタン押下検知
         * @retval true  押下
         * @retval false 未押下
         */
        bool inline isPressed()
        {
            return _previous;
        }

    private:
        bool _previous = false;
        bool _isRising = false;
        bool _isFalling = false;
};