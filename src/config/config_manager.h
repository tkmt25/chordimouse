#pragma once

#include <utils/internal_fs.h>

#include <config/config.h>
#include <config/key_profile.h>
#include <config/calibration.h>
#include <alias.h>
#include <utils/debug.h>

class config_manager
{

public:
    config_manager() = delete;

    static inline config& getGlobalConfig()
    {
        return _config;
    }

    static inline key_profiles& getKeyProfiles()
    {
        return _keyProfiles;
    }

    static inline calibration& getCalibration()
    {
        return _calibration;
    }

    static inline void init()
    {
        // 初回は失敗するのでデフォルト値を設定して保存しておく
        if (!loadFrom(CONFIG_FILENAME, _config)) 
        {
            _config = DEFAULT_CONFIG;
            saveConfig(_config);
        }
        if (!loadFrom(KEY_PROFILE_FILENAME, _keyProfiles)) 
        {
            _keyProfiles[0] = DEFAULT_KEY_PROFILES[0];
            _keyProfiles[1] = DEFAULT_KEY_PROFILES[1];

            saveKeyProfiles(_keyProfiles);
        }

        if (!loadFrom(CALIBRATION_FILENAME, _calibration)) 
        {
            // 現在値で校正
            calibration calib {{}, joystick::getX(), joystick::getY()};
            saveCalibration(calib);
        }
    }

    static inline void saveConfig(const config& config) { saveTo(CONFIG_FILENAME, config, _config); }
    static inline void saveKeyProfiles(const key_profiles& profs)  { saveTo(KEY_PROFILE_FILENAME, profs, _keyProfiles); }
    static inline void saveCalibration(const calibration& calib) { saveTo(CALIBRATION_FILENAME, calib, _calibration); } 

private:
    static inline config _config{};
    static inline key_profiles _keyProfiles{};
    static inline calibration _calibration{{}, 0, 0};

    static config DEFAULT_CONFIG;
    static key_profile DEFAULT_KEY_PROFILES[2];

    constexpr static char CONFIG_FILENAME[] = "/config";
    constexpr static char CALIBRATION_FILENAME[] = "/calib";
    constexpr static char KEY_PROFILE_FILENAME[] = "/key_profiles";

    template <typename T>
    static inline bool loadFrom(const char* filename, T& out)
    {
        auto opt = fs::load<T>(filename);
        if (!opt.has_value()) {
            return false;
        }
        out = std::move(opt.value());
        return true;
    }

    template <typename T>
    static inline void saveTo(const char* filename, const T& obj, T& out)
    {
        out = obj;
        fs::save(filename, obj);
    }
};