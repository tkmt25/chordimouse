#pragma once

#include <stdint.h>
#include <utils/serializable.h>
#include <string.h>

struct calibration : serializable<calibration>
{
    uint32_t centerX = 0;
    uint32_t centerY = 0;

    /**
     * @brief シリアライズ時のサイズを取得
     * @return サイズ
     */
    uint16_t inline getSerializedSize() const
    {
        return sizeof(calibration);
    }

    /**
     * @brief シリアライズする
     * @param [out] buffer 出力バッファ
     * @note 出力バッファのサイズはgetSerializedSize()で取得
     */
    uint16_t inline serialize(uint8_t *buffer) const
    {
        auto size = getSerializedSize();
        memcpy(buffer, this, size);
        return size;
    }


    /**
     * @brief デシリアライズする
     * @param [in] buffer 入力バッファ
     */
    bool inline deserialize(const uint8_t *buffer, const uint16_t buffSize)
    {
        auto size = getSerializedSize();
        if (buffSize < size) {
            return false;
        }

        calibration carib;
        memcpy(&carib, buffer, size);
        *this = carib;
        return true;
    }
};