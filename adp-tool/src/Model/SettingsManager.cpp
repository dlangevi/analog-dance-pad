#include "Adp.h"

#include "Model/SettingsManager.h"

#include <wx/config.h>
#include <wx/colour.h>

using namespace std;

namespace adp {

// Color settings
void SettingsManager::SetColor(const wxString& key, const wxColour& color) const {
    config->Write(key, color.GetAsString(wxC2S_HTML_SYNTAX));
    config->Flush();
}

wxColour SettingsManager::GetColor(const wxString& key, const wxColour& defaultColor) const {
    wxString colorStr;
    if (config->Read(key, &colorStr)) {
        wxColour color;
        if (color.Set(colorStr)) return color;
    }
    return *wxBLACK;  // Default
}

// Window position/size
void SettingsManager::SaveWindowGeometry(wxTopLevelWindow* window) const {
    wxPoint pos = window->GetPosition();
    wxSize size = window->GetSize();
    
    config->Write("/Window/X", pos.x);
    config->Write("/Window/Y", pos.y);
    config->Write("/Window/Width", size.x);
    config->Write("/Window/Height", size.y);
    config->Write("/Window/Maximized", window->IsMaximized());
    config->Flush();
}

void SettingsManager::RestoreWindowGeometry(wxTopLevelWindow* window) const {
    int x, y, width, height;
    bool maximized;
    
    if (config->Read("/Window/X", &x) &&
        config->Read("/Window/Y", &y) &&
        config->Read("/Window/Width", &width) &&
        config->Read("/Window/Height", &height)) {
        
        window->SetSize(x, y, width, height);
    }
    
    if (config->Read("/Window/Maximized", &maximized) && maximized) {
        window->Maximize();
    }
}

// Generic settings
void SettingsManager::SetString(const wxString& key, const wxString& value) const {
    config->Write(key, value);
    config->Flush();
}

wxString SettingsManager::GetString(const wxString& key, const wxString& defaultValue) const {
    wxString value;
    config->Read(key, &value, defaultValue);
    return value;
}

void SettingsManager::SetInt(const wxString& key, int value) const {
    config->Write(key, value);
    config->Flush();
}

int SettingsManager::GetInt(const wxString& key, int defaultValue) const {
    int value;
    config->Read(key, &value, defaultValue);
    return value;
}

void SettingsManager::SetBool(const wxString& key, bool value) const {
    config->Write(key, value);
    config->Flush();
}

bool SettingsManager::GetBool(const wxString& key, bool defaultValue) const {
    bool value;
    config->Read(key, &value, defaultValue);
    return value;
}

}; // namespace adp.
