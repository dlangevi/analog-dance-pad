#include <Adp.h>

#include <imgui.h>
#include <fmt/core.h>

#include <Model/Device.h>
#include <View/Colors.h>
#include <View/MappingTab.h>

namespace adp {

static string ButtonIndexToText(int index)
{
    return (index == 0) ? "-" : fmt::format("Button {}", index);
}

static void RenderSensorBar(int sensorIndex)
{
    auto sensorBarId = fmt::format("##SensorMappingBar{}", sensorIndex);
    ImGui::BeginChild(sensorBarId.data(), ImVec2(200, ImGui::GetFrameHeight()));

    auto wdl = ImGui::GetWindowDrawList();
    auto cp = ImGui::GetCursorScreenPos();
    auto cra = ImGui::GetContentRegionAvail();

    auto sensor = Device::Sensor(sensorIndex);
    auto fillW = sensor ? float(sensor->value) * cra.x : 0.f;

    wdl->AddRectFilled(
        { cp.x, cp.y },
        { cp.x + cra.x, cp.y + cra.y },
        RgbColorf::SensorBar.ToU32());

    wdl->AddRectFilled(
        { cp.x, cp.y },
        { cp.x + fillW, cp.y + cra.y },
        RgbColorf::SensorOff.ToU32());

    ImGui::EndChild();
}

void MappingTab::Render()
{
    auto pad = Device::Pad();
    bool configButton = Device::Pad()->featureDigipot;

    string buttonOptions("-\0", 2);
    for (int i = 1; i <= pad->numButtons; ++i)
        buttonOptions.append(fmt::format("Button {}{}", i, '\0'));

    for (int i = 0; i < pad->numSensors; ++i)
    {
        ImGui::BeginChild(
            fmt::format("SensorMappingChild{}", i).data(), 
            ImVec2(0, ImGui::GetFrameHeight()));
        auto sensorText = fmt::format("Sensor {}", i);
        int selected = Device::Sensor(i)->button;
        ImGui::TextUnformatted(sensorText.data());
        ImGui::SameLine();
        ImGui::SetCursorPosX(100);

        auto sensorId = fmt::format("##SensorMapping{}", i);
        ImGui::PushItemWidth(200);
        if (ImGui::Combo(sensorId.data(), &selected, buttonOptions.data()))
            Device::SetButtonMapping(i, selected);

        ImGui::SameLine();
        RenderSensorBar(i);
        ImGui::EndChild();

        if (configButton)
        {
            // auto configButton = new wxButton(this, i, "Config");
            // configButton->Bind(wxEVT_BUTTON, &MappingTab::OnSensorConfig, this);
            // sizer->Add(configButton, 1, wxEXPAND);
        }
    }
}

}; // namespace adp.
