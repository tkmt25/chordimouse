#pragma once

#include <stdint.h>
#include <InternalFileSystem.h>
#include <Adafruit_LittleFS.h>
#include <utils/serializable.h>
#include <vector>
#include <optional>
#include <utils/debug.h>

namespace fs
{
    /**
     * @brief 内蔵Flashにファイルを保存する
     * @param [in] filename ファイル名(フルパス)
     * @param [in] buffer    バッファ
     * @param [in] bufsize   バッファサイズ
     * @return 保存成否
     */
    bool save(const char* filename, const void* buffer, uint16_t bufsize);


    /**
     * @brief 内蔵Flashにファイルを保存する
     * @param [in] filename ファイル名(フルパス) 
     * @param [in] obj      保存対象
     * @return 保存成否
     */
    template <typename T>
    inline bool save(const char* filename, const serializable<T>& obj)
    {
        DEBUG_PRINTF(">> save %s", filename);
        uint16_t bufsize = obj.getSerializedSize();
        uint8_t buffer[bufsize];
        auto size = obj.serialize(buffer);
        auto success = save(filename, buffer, size);
        DEBUG_PRINTF(">> save %s", success ? "ok" : "ng");
        return success;
    }


    /**
     * @brief 内蔵Flashからファイルを読み込む
     * @param [in]  filename ファイル名(フルパス)
     * @param [out] buffer   バッファ
     */
    uint16_t load(const char* filename, void* buffer);


    /**
     * @brief 内蔵Flashからファイルを読み込む
     * @param [in]  filename ファイル名(フルパス)
     * @return 読み込んだデータ。失敗時はempty
     */
    std::vector<uint8_t> load(const char*filename);


    /**
     * @brief 内蔵Flashからファイルを読み込む
     * @param [in] filename ファイル名(フルパス)
     * @tparam T デシリアライズする型。deserializableのサブクラス
     * @return デシリアライズしたインスタンス
     */
    template <typename T>
    inline std::optional<T> load(const char* filename) 
    {
        DEBUG_PRINTF(">> load %s", filename);
        auto buff = fs::load(filename);
        if (buff.empty()) {
            DEBUG_PRINTF("empty");
            return std::nullopt;
        }

        T t;
        auto success = t.deserialize(buff.data(), buff.size());
        if (!success) {
            DEBUG_PRINTF("cannot deserialize");
            return std::nullopt;
        }

        DEBUG_PRINTF("<< load ");
        return t;
    }
}
