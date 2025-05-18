#pragma once

#include <string.h>
#include <initializer_list>
#include <unordered_map>
#include <layer/event.h>
#include <utils/serializable.h>

/*
 * keyboardプロファイル
 * unordered_mapはメモリ効率が気になるので後で最適化するかも
 */
class key_profile : public serializable<key_profile>
{
public:
    key_profile() = default;

    key_profile(std::initializer_list<std::pair<uint16_t, uint8_t>> list)
    {
        for (const auto &pair : list)
        {
            _map.insert(pair);
        }
    }

    void inline setChord(const uint16_t chord, const uint8_t scancode)
    {
        _map[chord] = scancode;
    }


    /**
     * @brief 指定したchordが存在するか確認します
     * @param [in] chord コード
     * @retval true  存在する
     * @retval false 存在しない
     */
    bool inline exists(const uint16_t chord) const
    {
        return _map.find(chord) != _map.end();
    }


    /**
     * @brief 指定したchordに対応するHIDスキャンコードを返却します
     * 
     * @param [in] chord コード
     * 
     * @retval 0      対応するスキャンコードが見つからない
     * @retval 0以外  スキャンコード
     */
    uint8_t inline getScancode(const uint16_t chord) const
    {
        if (!exists(chord)) {
            return 0;
        }
        return _map.at(chord);
    }


    /**
     * @brief シリアライズ時のサイズを取得
     * @return サイズ
     */
    uint16_t inline getSerializedSize() const
    {
        // [要素数] + [データ実体]
        return sizeof(uint16_t) + (ENTRY_SIZE * _map.size());
    }

    /**
     * @brief シリアライズする
     * @param [out] buffer 出力バッファ
     * @note 出力バッファのサイズはgetSerializedSize()で取得
     */
    uint16_t inline serialize(uint8_t *buffer) const
    {
        // 要素数
        uint8_t* offset = buffer;
        uint16_t count = _map.size();
        memcpy(offset, &count, sizeof(uint16_t));
        offset += sizeof(uint16_t);

        // データ実体
        for (const auto &pair : _map)
        {
            memcpy(offset, &pair.first, sizeof(uint16_t));
            offset += sizeof(uint16_t);
            memcpy(offset, &pair.second, sizeof(uint8_t));
            offset += sizeof(uint8_t);
        }
        return getSerializedSize();
    }


    /**
     * @brief デシリアライズする
     * @param [in] buffer 入力バッファ
     */
    bool inline deserialize(const uint8_t *buffer, const uint16_t buffSize)
    {
        auto totalSize = getSerializedSize();
        if (buffSize < totalSize) {
            return false;
        }

        // 要素数
        uint16_t count;
        uint8_t *p = const_cast<uint8_t*>(buffer);
        memcpy(&count, p, sizeof(uint16_t));
        p += sizeof(uint16_t);

        
        if (count * ENTRY_SIZE > (buffer + buffSize - p)) {
            return false;    
        }

        _map.clear();

        // データ実体
        for (auto i = 0; i < count; i++)
        {
            uint16_t chord;
            uint8_t scancode;
            memcpy(&chord, p, sizeof(uint16_t));
            p += sizeof(uint16_t);
            memcpy(&scancode, p, sizeof(uint8_t));
            p += sizeof(uint8_t);
            _map[chord] = scancode;
        }
        return true;
    }

    static constexpr int ENTRY_SIZE = sizeof(uint8_t) + sizeof(uint16_t);
    static constexpr int MAX_SIZE = sizeof(uint16_t) + (ENTRY_SIZE * 30); // 30/prof

private:
    std::unordered_map<uint16_t, uint8_t> _map;
};


/**
 * @brief 複数版
 */
class key_profiles : public serializable<key_profiles>
{
public:
    key_profile& operator[](const int i)
    {
        return _keyProfiles[i];
    }

    /**
     * @brief シリアライズ時のサイズを取得
     * @return サイズ
     */

    uint16_t inline getSerializedSize() const
    {
        auto totalSize = sizeof(uint8_t); // 先頭は要素数
        for (auto& c : _keyProfiles) {
            totalSize += c.getSerializedSize();
        }
        return totalSize;
    }

    /**
     * @brief シリアライズする
     * @param [out] buffer 出力バッファ
     * @note 出力バッファのサイズはgetSerializedSize()で取得
     */
    uint16_t inline serialize(uint8_t *buffer) const
    {
        uint8_t* p = buffer;

        // 先頭に要素数
        uint8_t count = _keyProfiles.size();
        memcpy(p, &count, sizeof(count));
        p += sizeof(count);

        for (const auto& prof : _keyProfiles) {
            p += prof.serialize(p);
        }

        return getSerializedSize();
    }


    /**
     * @brief デシリアライズする
     * @param [in] buffer 入力バッファ
     */
    bool inline deserialize(const uint8_t *buffer, const uint16_t buffSize)
    {
        uint8_t *p = const_cast<uint8_t*>(buffer);

        // 要素数
        uint8_t count;
        memcpy(&count, p, sizeof(count));
        p += sizeof(count);

        if (count > PROFILE_COUNT) {
            return false;
        }

        // 本体
        for (int i = 0; i < count; i++) {
            auto success = _keyProfiles[i].deserialize(p, buffSize - (p - buffer));
            if (!success) {
                return false;
            }
            p += _keyProfiles[i].getSerializedSize();
        }

        return true;
    }  

    static constexpr int PROFILE_COUNT = 2;
    static constexpr int MAX_SIZE = sizeof(uint8_t) + ( key_profile::MAX_SIZE * PROFILE_COUNT );

private:
    std::array<key_profile,PROFILE_COUNT> _keyProfiles;
};