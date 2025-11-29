#include <Adp.h>

#include <string>

#include <imgui.h>
#include <fmt/core.h>

#include <Model/Device.h>
#include <Model/Firmware.h>
#include <Model/Log.h>
#include <Model/Utils.h>

#include <View/DeviceTab.h>

using namespace std;

namespace adp {

static constexpr const char* SelectMsg =
    "Select the active pad device from the dropdown.";

static constexpr const char* RenameMsg =
    "Rename the pad device. Convenient if you have\nmultiple devices and want to tell them apart.";

static constexpr const char* RebootMsg =
    "Reboot to bootloader. This will restart the\ndevice and provide a window to update firmware.";

static constexpr const char* FactoryResetMsg =
    "Perform a factory reset. This will load the\nfirmware default configuration.";

static constexpr const char* UpdateFirmwareMsg =
    "Upload a firmware file to the pad device.";

static void OnUploadFirmware()
{
    /*
    wxFileDialog dlg(this, L"Open XYZ file", L"", L"", L"ADP firmware (*.hex)|*.hex",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    firmwareDialog->UpdateFirmware((dlg.GetPath().ToStdWstring()));
    */
}

void DeviceTab::Render()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

    ImGui::TextUnformatted(SelectMsg);

    auto devices = Device::GetConnectedPads();
    string buttonOptions = "";
    int selected = 0;
    for (int i = 0; i < devices.size(); i++) 
    {
        auto& [deviceName, devicePath] = devices[i];
        if (devicePath == Device::PadPath())
            selected = i;
        buttonOptions.append(fmt::format("{}{}", deviceName, '\0'));
    }

    if (ImGui::Combo("Pad Selection", &selected, buttonOptions.data()))
    {
        auto& [deviceName, devicePath] = devices[selected];
        Device::SetActivePad(devicePath);
    }

    static bool show_modal = false;
    ImGui::TextUnformatted(RenameMsg);
    if (ImGui::Button("Rename...", { 200, 30 }))
    {
      ImGui::OpenPopup("RenameDevice");
      show_modal = true;
    }

    // ImGui version of device renaming dialog
    if (ImGui::BeginPopupModal(
          "RenameDevice", 
          &show_modal, 
          ImGuiWindowFlags_AlwaysAutoResize)) {

        auto pad = Device::Pad();
        if (pad == nullptr)
            return;

        // Enought for max name + 1 for null terminator
        static char nameBuffer[MAX_NAME_LENGTH + 1]; 

        static bool initialized = false;
        if (!initialized) {
            strncpy(nameBuffer, pad->name.c_str(), sizeof(nameBuffer) - 1);
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';
            initialized = true;
        }

        ImGui::Text("Enter a new name for the device:");
        ImGui::InputText("##DeviceName", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_CallbackCharFilter,
            [](ImGuiInputTextCallbackData* data) {
                // Allow only a-z, A-Z, -, _, (, ), and space
                char c = data->EventChar;

                bool isLetter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
                bool isAllowedSymbol = c == '-' || c == '_' || c == '(' || c == ')' || c == ' ';
                if (isLetter || isAllowedSymbol) {
                    return 0;
                }
                return 1;
            });
        if (ImGui::Button("OK")) {
            Device::SetDeviceName(nameBuffer);
            initialized = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            initialized = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
    ImGui::TextUnformatted(FactoryResetMsg);
    if (ImGui::Button("Factory reset", { 200, 30 }))
        Device::SendFactoryReset();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
    ImGui::TextUnformatted(RebootMsg);
    if (ImGui::Button("Bootloader mode", { 200, 30 }))
        Device::SendDeviceReset();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
    ImGui::TextUnformatted(UpdateFirmwareMsg);
    if (ImGui::Button("Update firmware...", { 200, 30 }))
        OnUploadFirmware();

    ImGui::PopStyleVar();
}

}; // namespace adp.
