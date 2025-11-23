#include <Adp.h>

#include <map>
#include <vector>

#include <imgui.h>
#include <fmt/core.h>

#include <Model/Device.h>
#include <View/Colors.h>
#include <View/SensitivityTab.h>

#include <iostream>

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
    auto cra = ImGui::GetContentRegionAvail();
    auto wdl = ImGui::GetWindowDrawList();

    auto sensor = Device::Sensor(sensorIndex);
    auto pressed = sensor ? sensor->pressed : false;

    auto threshold = sensor ? sensor->threshold : 0.0;
    if (myAdjustingSensorIndex == sensorIndex)
        threshold = myAdjustingSensorThreshold;

    auto fillH = sensor ? float(sensor->value) * cra.y : 0.f;
    float thresholdY = float(1 - threshold) * cra.y;

    // Full bar.
    wdl->AddRectFilled(
        { csp.x, csp.y },
        { csp.x + cra.x, csp.y + cra.y },
        RgbColorf::SensorBar.ToU32());

    // Filled bar that indicates current sensor reading.
    wdl->AddRectFilled(
        { csp.x, csp.y + cra.y - fillH },
        { csp.x + cra.x, csp.y + cra.y },
        pressed ? RgbColorf::SensorOn.ToU32() : RgbColorf::SensorOff.ToU32());

    // Line representing where the release threshold would be for the current sensor.
    if (myReleaseThreshold < 1.0)
    {
        float releaseY = float(1 - myReleaseThreshold * threshold) * cra.y;
        wdl->AddRectFilled(
            { csp.x , csp.y + thresholdY },
            { csp.x + cra.x, csp.y + thresholdY + max(1.f, releaseY - thresholdY) },
            IM_COL32(50, 50, 50, 100));
    }

    // Line representing the sensitivity threshold.
    wdl->AddRectFilled(
        { csp.x , csp.y + thresholdY - 2 },
        { csp.x + cra.x, csp.y + thresholdY + 1 },
        IM_COL32_BLACK);
    wdl->AddRectFilled(
        { csp.x , csp.y + thresholdY - 1 },
        { csp.x + cra.x, csp.y + thresholdY },
        IM_COL32_WHITE);

    // Small text block at the top displaying sensitivity threshold.
    auto thresholdStr = fmt::format("{}%", (int)std::lround(threshold * 100.0));
    auto ts = ImGui::CalcTextSize(thresholdStr.data());
    ImGui::SetCursorPosX((cra.x - ts.x) / 2);
    ImGui::TextUnformatted(thresholdStr.data());
    ImGui::TextUnformatted(pressed ? "Pressed" : "Released");

    // Start/finish sensor threshold adjusting based on LMB click/release.
    if (myAdjustingSensorIndex == SENSOR_INDEX_NONE)
    {
        if (ImGui::IsMouseClicked(ImGuiPopupFlags_MouseButtonLeft) && ImGui::IsMousePosValid())
        {
            auto pos = ImGui::GetMousePos();
            if (pos.x >= csp.x && pos.x < csp.x + cra.x &&
                pos.y >= csp.y && pos.y < csp.y + cra.y)
            {
                myAdjustingSensorIndex = sensorIndex;
                myAdjustingSensorThreshold = 0.0;
                ImGui::CaptureMouseFromApp(true);
            }
        }
    }
    else if (!ImGui::IsMouseDown(ImGuiPopupFlags_MouseButtonLeft))
    {
        Device::SetThreshold(myAdjustingSensorIndex, myAdjustingSensorThreshold);
        myAdjustingSensorIndex = SENSOR_INDEX_NONE;
    }

    // If sensor threshold adjusting is active, update threshold based on mouse position.
    if (myAdjustingSensorIndex == sensorIndex)
    {
        double value = double(ImGui::GetMousePos().y) - (csp.y);
        double range = max(1.0, double(cra.y));
        myAdjustingSensorThreshold = clamp(1.0 - (value / range), 0.0, 1.0);
    }
}

void SensitivityTab::Render()
{
    auto cra = ImGui::GetContentRegionAvail();

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
    cra = ImGui::GetContentRegionAvail();
    auto pos = ImGui::GetCursorScreenPos();
    float colW = (cra.x / sensors) - 5;
    float colH = cra.y;

    // ImGui::BeginGroup();
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
    // ImGui::EndGroup();
    ImGui::EndChild();

    if (myReleaseThreshold < 0.0f && Device::Pad())
    {
        myReleaseThreshold = Device::Pad()->releaseThreshold;
    }

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
    ImGui::SliderFloat(
        "##ReleaseThreshold",
        &myReleaseThreshold,
        0.01f,
        1.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp
    );
    ImGui::PopItemWidth();

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        Device::SetReleaseThreshold(myReleaseThreshold);
    }

    ImGui::EndChild();
}

}; // namespace adp.
