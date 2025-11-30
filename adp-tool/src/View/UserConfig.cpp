#include <View/UserConfig.h>

#include <filesystem>
#include <fstream>
#include <sago/platform_folders.h>
#include <nlohmann/json.hpp>

namespace adp {

bool UserConfig::Init() {
  configDir = std::filesystem::path(sago::getConfigHome()) / "adp_tool";

  if (!std::filesystem::exists(configDir)) {
    std::filesystem::create_directories(configDir);
  }

  configPath = configDir /  "config.json";
  return UserConfig::LoadFromDisk();
}

RgbColorf ReadColor(const nlohmann::json& j, const std::string& key, const RgbColorf& defaultColor) {
  if (j.contains(key)) {
    auto value = j[key];
    if (value.is_array() && value.size() == 3) {
      float r = value[0];
      float g = value[1];
      float b = value[2];
      return RgbColorf(r, g, b);
    }
  }
  return defaultColor;
}

void WriteColor(nlohmann::json& j, const std::string& key, const RgbColorf& color) {
  j[key] = { color.rgb[0], color.rgb[1], color.rgb[2] };
}


bool UserConfig::LoadFromDisk() {

  nlohmann::json j;
  if (std::filesystem::exists(configPath)) {
    std::ifstream inputStream(configPath);
    inputStream >> j;
    inputStream.close();

    UserConfig::SensorOn = ReadColor(j, "SensorOn", UserConfig::SensorOn);
    UserConfig::SensorOff = ReadColor(j, "SensorOff", UserConfig::SensorOff);
    UserConfig::SensorBar = ReadColor(j, "SensorBar", UserConfig::SensorBar);
  }

  // Return if the Load succeeded
  return true;
}

bool UserConfig::SaveToDisk() {
  nlohmann::json j;

  WriteColor(j, "SensorOn", UserConfig::SensorOn);
  WriteColor(j, "SensorOff", UserConfig::SensorOff);
  WriteColor(j, "SensorBar", UserConfig::SensorBar);

  // Open file
  std::ofstream outputStream(configPath);
  if (!outputStream) {
    return false;
  }

  outputStream << j.dump(4);
  outputStream.close();
  return true;
}

std::filesystem::path UserConfig::configDir = "";
std::filesystem::path UserConfig::configPath = "";

RgbColorf UserConfig::SensorOn = RgbColorf(0.98f, 0.902f, 0.745f);
RgbColorf UserConfig::SensorOff = RgbColorf(0.902f, 0.627f, 0.392f);
RgbColorf UserConfig::SensorBar = RgbColorf(0.098f, 0.098f, 0.098f);

} // namespace adp.
