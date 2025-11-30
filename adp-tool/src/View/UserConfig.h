#pragma once

#include "View/Colors.h"

namespace adp {

class UserConfig {
  public:
    static bool Init();

    static bool SaveToDisk();

    static bool LoadFromDisk();

    static RgbColorf SensorOn;
    static RgbColorf SensorOff;
    static RgbColorf SensorBar;

  private:
    static std::filesystem::path configDir;
    static std::filesystem::path configPath;
};

} // namespace adp
