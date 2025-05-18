#pragma once

#include <stdint.h>

/**
 * @brief シリアライズ可能を表すインターフェース
 */
template <typename clazz>
class serializable
{
    public:

        uint16_t getSerializedSize() const
        {
            return static_cast<const clazz *>(this)->getSerializedSize();
        }
        uint16_t serialize(uint8_t *buffer) const
        {
            return static_cast<const clazz *>(this)->serialize(buffer);
        }
        bool deserialize(const uint8_t *buffer, const uint16_t buffSize)
        {
            return static_cast<const clazz *>(this)->deserialize(buffer, buffSize);
        }
};