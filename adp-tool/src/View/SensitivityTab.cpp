#include <Adp.h>

#include <map>
#include <vector>

#include <imgui.h>
#include <fmt/core.h>

#include <Model/Device.h>
#include <View/Colors.h>
#include <View/SensitivityTab.h>

using namespace std;

namespace adp {

static const char* ActivationMsg =
    "Click inside a sensor bar to adjust its activation threshold.";

static const char* ReleaseMsg =
    "Adjust release threshold (percentage of activation threshold).";

static constexpr int SENSOR_INDEX_NONE = -1;

SensitivityTab::SensitivityTab()
    : myAdjustingSensorIndex(SENSOR_INDEX_NONE)
{
}

void SensitivityTab::RenderSensor(int sensorIndex)
{
    auto csp = ImGui::GetCursorScreenPos();
    auto size = ImGui::GetWindowSize();
    auto wdl = ImGui::GetWindowDrawList();

    // mouse is down on this item
    if (ImGui::IsItemActive()) {
        float mouseY = ImGui::GetIO().MousePos.y;
        float t = (mouseY - csp.y) / size.y;
        myAdjustingSensorIndex = sensorIndex;
        myAdjustingSensorThreshold = clamp(1.0f - t, 0.0f, 1.0f);
    }

    // mouse released on this item
    if (ImGui::IsItemDeactivated()) {
        Device::SetThreshold(sensorIndex, myAdjustingSensorThreshold);
    } 

    auto sensor = Device::Sensor(sensorIndex);
    auto pressed = sensor ? sensor->pressed : false;
    auto threshold = sensor ? sensor->threshold : 0.0;

    // If adjusting the sensor currently, override threshold with adjusting value.
    if (myAdjustingSensorIndex == sensorIndex) {
        threshold = myAdjustingSensorThreshold;
        pressed = sensor->value >= threshold;
    }

    auto fillH = sensor ? float(sensor->value) * size.y : 0.f;
    float thresholdY = float(1 - threshold) * size.y;

    // Full bar.
    wdl->AddRectFilled(
        { csp.x, csp.y },
        { csp.x + size.x, csp.y + size.y },
        RgbColorf::SensorBar.ToU32());

    // Filled bar that indicates current sensor reading.
    wdl->AddRectFilled(
        { csp.x, csp.y + size.y - fillH },
        { csp.x + size.x, csp.y + size.y },
        pressed ? RgbColorf::SensorOn.ToU32() : RgbColorf::SensorOff.ToU32());

    // Line representing where the release threshold would be for the current sensor.
    if (myReleaseThreshold < 1.0)
    {
        float releaseY = float(1 - myReleaseThreshold * threshold) * size.y;
        wdl->AddRectFilled(
            { csp.x , csp.y + thresholdY },
            { csp.x + size.x, csp.y + thresholdY + max(1.f, releaseY - thresholdY) },
            IM_COL32(50, 50, 50, 100));
    }

    // Line representing the sensitivity threshold.
    wdl->AddRectFilled(
        { csp.x , csp.y + thresholdY - 2 },
        { csp.x + size.x, csp.y + thresholdY + 1 },
        IM_COL32_BLACK);
    wdl->AddRectFilled(
        { csp.x , csp.y + thresholdY - 1 },
        { csp.x + size.x, csp.y + thresholdY },
        IM_COL32_WHITE);

    // Small text block at the top displaying sensitivity threshold.
    auto thresholdStr = fmt::format("{}%", (int)std::lround(threshold * 100.0));
    auto ts = ImGui::CalcTextSize(thresholdStr.data());
    ImGui::SetCursorPosX((size.x - ts.x) / 2);
    ImGui::TextUnformatted(thresholdStr.data());
}

void SensitivityTab::Render()
{
    // Cannot depend on pad being avalibe at construction time.
    if (myReleaseThreshold < 0 && Device::Pad())
    {
        myReleaseThreshold = Device::Pad()->releaseThreshold;
    }

    auto size = ImGui::GetWindowSize();

    int colorEditFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
    ImGui::TextUnformatted(ActivationMsg);
    ImGui::SameLine();
    ImGui::ColorEdit3("SensorBar", RgbColorf::SensorBar.rgb, colorEditFlags);
    ImGui::SameLine();
    ImGui::ColorEdit3("SensorOn", RgbColorf::SensorOn.rgb, colorEditFlags);
    ImGui::SameLine();
    ImGui::ColorEdit3("SensorOff", RgbColorf::SensorOff.rgb, colorEditFlags);

    map<int, vector<int>> mappings; // button -> sensors[]
    int numMappedSensors = 0;

    auto pad = Device::Pad();
    if (pad)
    {
        for (int i = 0; i < pad->numSensors; ++i)
        {
            auto sensor = Device::Sensor(i);
            if (sensor->button != 0)
            {
                mappings[sensor->button].push_back(i);
                ++numMappedSensors;
            }
        }
    }

    // Need to leave room at the bottom for the slider
    float avaliable = ImGui::GetContentRegionAvail().y - 100;
    ImGui::BeginChild("SensitivitySensors", ImVec2(0, avaliable), false, ImGuiWindowFlags_None);
    auto sensors = max(numMappedSensors, 1);

    // We want padding of 5 on all sides and between columns.
    size = ImGui::GetContentRegionAvail();
    float colW = (size.x / sensors) - 5;
    float colH = size.y;

    ImGui::BeginGroup();
    for (auto& button : mappings)
    {
        for (auto& sensor : button.second)
        {
            ImGui::BeginChild(
                fmt::format("SensitivitySensorChild{}", sensor).data(),
                ImVec2(colW, colH));
            RenderSensor(sensor);
            ImGui::EndChild();
            ImGui::SameLine();
        }
    }
    ImGui::EndGroup();
    ImGui::EndChild();

    ImGui::BeginChild(
      "ReleaseThresholdSlider",
      ImVec2(0, 0),
      false,
      ImGuiWindowFlags_None);

    float sliderWidth = 400.0f;

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float approxSize = ImGui::GetFrameHeight() * 2.0f; // text + slider

    // Compute centered position
    float posX = (avail.x - sliderWidth) * 0.5f;
    float posY = (avail.y - approxSize) * 0.5f;

    // Move cursor to that centered spot
    ImGui::SetCursorPos(ImVec2(posX, posY));

    // Draw ReleaseMsg
    auto ts = ImGui::CalcTextSize(ReleaseMsg);
    ImGui::SetCursorPosX( (avail.x - ts.x) / 2 );
    ImGui::TextUnformatted(ReleaseMsg);

    // Draw Slider
    ImGui::SetCursorPosX(posX);
    ImGui::PushItemWidth(sliderWidth);
    {
        ImGui::SliderFloat(
            "##ReleaseThreshold",
            &myReleaseThreshold,
            0.01f,
            1.0f,
            "%.2f",
            ImGuiSliderFlags_AlwaysClamp
        );
    }
    ImGui::PopItemWidth();

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        // Only set if its a real value.
        if (myReleaseThreshold >= 0.0f)
        {
          Device::SetReleaseThreshold(myReleaseThreshold);
        }
    }

    ImGui::EndChild();
}

}; // namespace adp.
