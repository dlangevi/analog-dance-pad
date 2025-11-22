#pragma once

#include <wx/window.h>
#include <wx/toplevel.h>
#include <wx/config.h>
#include <wx/colour.h>

namespace adp {

class SettingsManager {
public:
    static SettingsManager& Get() {
        static SettingsManager instance;
        return instance;
    }

    void SetColor(const wxString& key, const wxColour& color) const;
    wxColour GetColor(const wxString& key, const wxColour& defaultColor) const;

    void SaveWindowGeometry(wxTopLevelWindow* window) const;
    void RestoreWindowGeometry(wxTopLevelWindow* window) const;

    void SetString(const wxString& key, const wxString& value) const;
    wxString GetString(const wxString& key, const wxString& defaultValue) const;

    void SetInt(const wxString& key, int value) const;
    int GetInt(const wxString& key, int defaultValue) const;

    void SetBool(const wxString& key, bool value) const;
    bool GetBool(const wxString& key, bool defaultValue) const;

private:
    SettingsManager() {
        config = wxConfigBase::Get();
    }
    
    wxConfigBase* config;

};

};
