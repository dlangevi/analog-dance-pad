#pragma once

#include "wx/window.h"
#include "wx/sizer.h"
#include "wx/stattext.h"
#include "wx/gauge.h"
#include "wx/choice.h"

#include "View/BaseTab.h"

using namespace std;

namespace adp {

class FirmwareDialog : public wxDialog
{
public:
    FirmwareDialog(const wxString& title);

    void UpdateFirmware(wstring file);

private:
    void OnAvrdude(wxCommandEvent& event);
    void SetStatus(wxString status);
    void Done();

    wxStaticText* statusText;
    wxGauge* progressBar;
    FirmwareUploader uploader;
    int tasksCompleted;
    int tasksTodo;

    DECLARE_EVENT_TABLE()
};

class DeviceTab : public BaseTab, public wxWindow
{
public:
    static const wchar_t* Title;
    FirmwareDialog* firmwareDialog;

    DeviceTab(wxWindow* owner);

    void OnPadSelected(wxCommandEvent &event);
    void OnRename(wxCommandEvent& event);
    void OnReboot(wxCommandEvent& event);
    void OnFactoryReset(wxCommandEvent& event);
    void OnUploadFirmware(wxCommandEvent& event);
    void PopulateDeviceList();
    void OnDeviceSelection(wxCommandEvent& event);

    wxWindow* GetWindow() override { return this; }

    DECLARE_EVENT_TABLE()

private:
    wxChoice* deviceChoice;
};

}; // namespace adp.
