#include <Adp.h>

#include <View/Colors.h>

using namespace std;

namespace adp {

RgbColorf::RgbColorf()
{
	rgb[0] = rgb[1] = rgb[2] = 0.f;
}

RgbColorf::RgbColorf(const RgbColor& color)
{
	rgb[0] = color.red * (1.f / 255);
	rgb[1] = color.green * (1.f / 255);
	rgb[2] = color.blue * (1.f / 255);
}

RgbColorf::RgbColorf(float r, float g, float b)
{
	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

ImU32 RgbColorf::ToU32() const
{
	return ImGui::ColorConvertFloat4ToU32(ImVec4(rgb[0], rgb[1], rgb[2], 1.0f));
}

RgbColor RgbColorf::ToRGB() const
{
	return RgbColor(
		uint8_t(clamp(int(rgb[0] * 255.f), 0, 255)),
		uint8_t(clamp(int(rgb[1] * 255.f), 0, 255)),
		uint8_t(clamp(int(rgb[2] * 255.f), 0, 255)));
}

RgbColorf RgbColorf::SensorOn(0.98f, 0.902f, 0.745f);
RgbColorf RgbColorf::SensorOff(0.902f, 0.627f, 0.392f);
RgbColorf RgbColorf::SensorBar(0.098f, 0.098f, 0.098f);
RgbColorf RgbColorf::GraphActive(0.116f, 0.936f, 0.003f);

// MAX_SENSOR_COUNT colors picked by Copilot 
vector<RgbColorf> RgbColorf::GraphColors = {
  RgbColorf(0.91f, 0.30f, 0.24f),
  RgbColorf(0.18f, 0.80f, 0.44f),
  RgbColorf(0.20f, 0.60f, 0.86f),
  RgbColorf(0.61f, 0.35f, 0.71f),
  RgbColorf(1.00f, 0.50f, 0.00f),
  RgbColorf(0.56f, 0.56f, 0.56f),
  RgbColorf(0.20f, 0.80f, 0.60f),
  RgbColorf(0.90f, 0.49f, 0.13f),
  RgbColorf(0.17f, 0.63f, 0.17f),
  RgbColorf(0.61f, 0.15f, 0.69f),
  RgbColorf(0.20f, 0.60f, 0.86f),
  RgbColorf(0.91f, 0.30f, 0.24f),
};

}; // namespace adp.
