/**
 * @brief カーソル移動のアルゴリズム
 * 
 */
#pragma once

#include <stdint.h>
#include <math.h>
#include <utils/axis_detector.h>
#include <utils/debug.h>

/**
 * @brief カーソル移動量
 */
typedef struct {
    int8_t x; ///< X軸
    int8_t y; ///< Y軸
} MoveCursor;


/**
 * @brief 移動距離
 */
struct Distance{
    float x; ///< X軸
    float y; ///< Y軸

    constexpr Distance& operator+(const Distance& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    constexpr Distance& operator-(const Distance& rhs)
    {
        y -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    constexpr Distance& operator/(const uint32_t r) {
        x /= r;
        y /= r;
        return *this;
    }
};


/**
 * @brief 加速度
 */
struct Velocity
{
    float x; ///< X軸
    float y; ///< Y軸
};

/**
 * @brief サンプリングおよび移動距離計算を行うクラス
 */
template <typename strategy>
class sampler
{
    public:
        sampler(const uint32_t intervalMs) : _intervalMs(intervalMs)
        {}

        inline strategy& getStorategy() {
            return _strategy;
        }

        inline void setInterval(const uint32_t intervalMs) 
        {
            _intervalMs = intervalMs;
        }

        inline void reset() {
            _lastSampledTimeUs = micros();
            _lastIntervalMs = millis();
        }

        inline void update(const int32_t x, const int32_t y)
        {
            auto now = micros();
            auto velocity = _strategy.getVelocity(x, y);

            auto deltaT = (float)(now - _lastSampledTimeUs) / (1.0f * 1000 * 1000);
            
            // 加速度*Δt=距離を積算
            _assumedDistance.x += (velocity.x * deltaT);
            _assumedDistance.y += (velocity.y * deltaT);

            DEBUG_PRINTF("x %f y %f dt %f (%d)", _assumedDistance.x, _assumedDistance.y, deltaT, now - _lastSampledTimeUs);

            _lastSampledTimeUs = now;
        }

        inline MoveCursor getMoveCursor(const int32_t x, const int32_t y)
        {
            update(x, y);

            // intervalMsに満たない場合は積算のみでカーソル移動距離は0
            auto now = millis();
            if (now - _lastIntervalMs < _intervalMs) {
                return MoveCursor{0, 0};
            }

            auto move = MoveCursor{
                (int8_t)_assumedDistance.x,
                (int8_t)_assumedDistance.y
            };
            DEBUG_PRINTF("moved ! %d,%d", move.x, move.y);

            _lastIntervalMs = now;

            // 残差(小数部)は次回に持ち越し
            _assumedDistance.x -= (move.x);
            _assumedDistance.y -= (move.y);

            // 残差がたまりすぎるとドリフトするので毎フレーム薄める
            _assumedDistance.x *= 0.95f; 
            _assumedDistance.y *= 0.95f;
            return move;
        }


    private:
        Distance _assumedDistance = {0, 0};
        uint32_t _lastSampledTimeUs = 0;
        uint32_t _lastIntervalMs = 0;
        uint32_t _intervalMs;
        strategy _strategy;
};



/**
 * @brief 負の慣性伝達関数を使ったカーソル移動
 * @note US5570111(特許期限切れ)を参考にした
 */
class negative_inertia_strategy
{
    public:
        inline void setGain(const int gain)
        {
            _gain = gain;
        }

        inline void setMickeyScale(const float mickeyScale)
        {
            _mickeyScale = mickeyScale;
        }

        /**
         * @brief 加速度を計算
         * @param [in] moveX  X軸入力
         * @param [in] moveY  Y軸入力
         */
        inline Velocity getVelocity(const int32_t moveX, const int32_t moveY)
        {
            // -255 - 255の範囲に正規化
            constexpr int16_t RAW_MAX   = 512;   // センサ ±レンジ
            constexpr int16_t OUT_MAX   = 255;   // LUT 上限

            int32_t x = (int32_t(moveX) * OUT_MAX + RAW_MAX/2) / RAW_MAX;  // 四捨五入
            int32_t y = (int32_t(moveY) * OUT_MAX + RAW_MAX/2) / RAW_MAX;
            DEBUG_PRINTF("norm_x=%d, norm_y=%d", x, y);

            uint32_t z = calcMagnitude(x, y);
            if (z == 0) {
                return {0, 0};
            }

            if (!_initialized) {
                _z0 = z;
                _initialized = true;
            }

            auto zi = calcInertiaCorrectedMagnitude(z, _z0, _gain);

#if __DEBUG__            
            auto tmp = _z0;
#endif
            _z0 = z;

            uint32_t zi2 = abs(zi);
            float Z = transferFunction(zi2, _mickeyScale);
            DEBUG_PRINTF("z = %d, z0 = %d, zi = %d, zi2 = %d, Z = %f", z, tmp, zi, zi2, Z);
            return Velocity{
                calcVelocity(x, Z, zi, zi2),
                calcVelocity(y, Z, zi, zi2),
            };
        }

    private:
        constexpr static uint32_t Z_MAX = 255;

        uint32_t _z0 = 0; ///< 一つ前のZ
        bool _initialized = false;
        Distance  _distance;
        int _gain = 6;
        float _mickeyScale = 50.0f / Z_MAX;


        /**
         * @brief かかった力の大きさを計算
         * @param [in] x X入力
         * @param [in] y Y入力
         * @return 力の大きさ(Z)
         */
        inline uint32_t calcMagnitude(const int32_t x, const int32_t y)
        {
            int32_t ax = abs(x);
            int32_t ay = abs(y);
            uint32_t z = ax + ay - ((2 * min(ax, ay)) / 3);

            if (z > Z_MAX) return Z_MAX;
            return z;
        }


        /**
         * @brief 負の慣性を計算
         * @param [in] z  力の大きさ
         * @param [in] z0 ???
         * @return 負の慣性値
         */
        inline int32_t calcInertiaCorrectedMagnitude(const uint32_t z, const uint32_t z0, const int gain)
        {
            return  ((z - z0) * gain) + z;
        }


        /**
         * @brief mickeys/sec(移動数)を取得する伝達関数
         * @param [in] zi ???
         * @return mickeys/sec
         */
        inline float transferFunction(const uint32_t zi, const float scale) 
        {
            /*
             * | input(n) | output(mickeys/sec) |
             * |----------|---------------------|
             * | 0 - 3    | 0                   |
             * | 4 - 10   | 18*(100/256)        |
             * | 11 - 16  | 56*(100/256)        |
             * | 17 - 19  | (n-15)56*(100/256)  |
             * | 20 - 23  | (n-1)16*(100/256)   |
             * | 31 - 38  | (n-11)25*(100/256)  |
             * | 39 - 49  | 704*(100/256)       |
             * | 50 - 255 | (n-40)74*(100/256)  |
             */
            if (zi <= 3) return 0.0f;
            //else if (zi <=  6) return  8.0f * scale;
            else if (zi <= 10) return 18.0f * scale;
            else if (zi <= 16) return 56.0f * scale;
            else if (zi <= 19) return (zi - 15) * 56.0f * scale;
            else if (zi <= 30) return (zi - 1) * 16.0f * scale;
            else if (zi <= 38) return (zi - 11) * 25.0f * scale;
            else if (zi <= 49) return 704.0f * scale;
            else if (zi <= 200) return (zi - 40.0f) * 74.0f * scale;
            else { // 
                return zi * 80.0f * scale;
            }
        }

        
        /**
         * @brief 加速度
         * @param [in] move
         * @param [in] Z
         * @param [in] zi
         * @param [in] zi2
         */
        inline float calcVelocity(const int32_t move, const float Z, const int32_t zi, const uint32_t zi2)
        {
            if (zi2 == 0) return 0;
            float ratio =  (Z / static_cast<float>(zi2));
            float mickey = (move * (zi < 0 ? -ratio : ratio));
            return mickey;
        }
};
