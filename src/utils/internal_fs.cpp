#include <utils/internal_fs.h>

#include <InternalFileSystem.h>
#include <Adafruit_LittleFS.h>

using namespace Adafruit_LittleFS_Namespace;

namespace fs
{
    bool save(const char* filename, const void* buffer, uint16_t bufsize)
    {
        if (!InternalFS.begin()) {
            return false;
        }

        // InternalFSでは実装上必ず追記になるようなので最初に削除する
        if (InternalFS.exists(filename)) {
            InternalFS.remove(filename);
        }

        File file = InternalFS.open(filename, FILE_O_WRITE);
        if (!file) {
            return false;
        }
        
        auto size = file.write((const uint8_t*)buffer, bufsize);
        file.close();
        return size == bufsize;
    }

    
    uint16_t load(const char* filename, void* buffer)
    {
        if (!InternalFS.begin()) {
            return 0;
        }

        File file = InternalFS.open(filename, FILE_O_READ);
        if (!file) {
            return 0;
        }

        auto size = file.size();
        file.read((uint8_t*)buffer, size);
        file.close();
        return size;
    }

    std::vector<uint8_t> load(const char* filename)
    {
        DEBUG_PRINTF(">> load %s", filename);
        std::vector<uint8_t> buff;

        if (!InternalFS.begin()) {
            DEBUG_PRINTF("InternalFS.begin() failed");
            return buff;
        }

        File file = InternalFS.open(filename, FILE_O_READ);
        if (!file) {
            DEBUG_PRINTF("open file failed");
            return buff;
        }
        
        auto size = file.size();
        buff.resize(size);
        file.read((uint8_t*)buff.data(), size);
        file.close();
        DEBUG_PRINTF("<< load");
        return buff;
    }
}