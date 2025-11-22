#include "Adp.h"
#include "Main.h"

#include <iostream>
#include <vector>
#include <fstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;
 
#include <wx/config.h> 
#include "wx/setup.h"
#include "wx/wx.h"
#include "wx/notebook.h"
#include "wx/wfstream.h"
#include "wx/sstream.h"
#include "wx/filename.h"
#include "wx/toolbar.h"
#include "wx/colordlg.h"

#include "Assets/Assets.h"

#include "View/BaseTab.h"
#include "View/IdleTab.h"
#include "View/SensitivityTab.h"
#include "View/GraphTab.h"
#include "View/MappingTab.h"
#include "View/LightsTab.h"
#include "View/DeviceTab.h"
#include "View/AboutTab.h"
#include "View/LogTab.h"

#include "Model/Log.h"
#include "Model/Updater.h"
#include "Model/SettingsManager.h"
#include "View/UpdaterView.h"

namespace adp {

enum Ids { PROFILE_LOAD = 1, PROFILE_SAVE = 2, MENU_EXIT = 3};

enum
{
  IDM_TOOLBAR_TOGGLE_TOOLBAR = 200,
};

// ====================================================================================================================
// Main window.
// ====================================================================================================================

class MainWindow : public wxFrame
{
public:
    MainWindow(wxApp* app, const wchar_t* versionString)
        : wxFrame(nullptr, wxID_ANY, TOOL_NAME, wxDefaultPosition, wxSize(500, 500))
        , myApp(app)
    {
        SetMinClientSize(wxSize(400, 400));
        SetStatusBar(CreateStatusBar(2));

        wxMenuBar* menuBar = new wxMenuBar();
        wxMenu* fileMenu = new wxMenu();

        menuBar->Append(fileMenu, wxT("File"));

        fileMenu->Append(PROFILE_LOAD, wxT("Load profile"));
        fileMenu->Append(PROFILE_SAVE, wxT("Save profile"));
        fileMenu->Append(MENU_EXIT, wxT("Exit"));

        wxMenu* toolBarMenu = new wxMenu();
        menuBar->Append(toolBarMenu, wxT("Toolbar"));
        toolBarMenu->AppendCheckItem(IDM_TOOLBAR_TOGGLE_TOOLBAR,
                              "Toggle &toolbar\tCtrl-Z",
                              "Show or hide the toolbar");

        SetMenuBar(menuBar);

        auto sizer = new wxBoxSizer(wxVERTICAL);
        myTabs = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_NOPAGETHEME);
        AddTab(0, new AboutTab(myTabs, versionString), AboutTab::Title);
        AddTab(1, new LogTab(myTabs), LogTab::Title);
        sizer->Add(myTabs, 1, wxEXPAND);
        SetSizer(sizer);
        UpdatePages();

        myUpdateTimer = make_unique<UpdateTimer>(this);
        myUpdateTimer->Start(10);
    }

    ~MainWindow()
    {
        myUpdateTimer->Stop();
    }

    void ProfileLoad(wxCommandEvent & event)
    {
		if(!Device::Pad()) {
			return;
		}

        wxFileDialog dlg(this, L"Load ADP profile", L"", lastProfile, L"ADP profile (*.json)|*.json|All files (*)|*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (dlg.ShowModal() == wxID_CANCEL)
            return;

		ifstream fileStream;
		fileStream.open((std::string)dlg.GetPath());
		if(!fileStream.is_open())
		{
			Log::Writef(L"Could not read profile: %ls", dlg.GetPath());
            return;
        }

        try {
            lastProfile = dlg.GetPath();

            json j;

            fileStream >> j;
            fileStream.close();

            Device::LoadProfile(j, DGP_ALL);
		} catch (exception e) {
            Log::Writef(L"Could not read profile: %hs", e.what());
		}
    }

    void ProfileSave(wxCommandEvent & event)
    {
		if(!Device::Pad()) {
			return;
		}

		wxString path = "";
		if(lastProfile.Length() > 0) {
            path = wxFileName(lastProfile).GetPath();
		}

        wxFileDialog dlg(this, L"Save ADP profile", path, L"profile", L"ADP profile (*.json)|*.json|All files (*)|*",
        wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

        if (dlg.ShowModal() == wxID_CANCEL)
            return;

        wxFileOutputStream output_stream(dlg.GetPath());
        if (!output_stream.IsOk())
        {
			Log::Writef(L"Could not save profile: %ls", dlg.GetPath());
            return;
        }

        try {
            lastProfile = dlg.GetPath();

            json j;

            Device::SaveProfile(j, DGP_ALL);

            wxStringInputStream input_stream(wxString(j.dump(4)));
            output_stream.Write(input_stream);
            output_stream.Close();
        } catch (exception e) {
            Log::Writef(L"Could not save profile: %hs", e.what());
        }
    }

    void Tick()
    {
        auto changes = Device::Update();

        if (changes & DCF_DEVICE) {
            UpdatePages();
            /*
            Updater::CheckForFirmwareUpdates([](SoftwareUpdate& update) {
                ShowUpdateDialog(update);
            });
            */
        }

        if (changes & (DCF_DEVICE | DCF_NAME))
            UpdateStatusText();

        wstring debugMessage = Device::ReadDebug();

        if (!debugMessage.empty()) {
            Log::Writef(L"\\/ \\/ \\/ Debug \\/ \\/ \\/\n%ls", debugMessage.c_str());
        }

        UpdatePollingRate();

        if (changes)
        {
            for (auto tab : myTabList)
                tab->HandleChanges(changes);
        }

        auto activeTab = GetActiveTab();
        if (activeTab)
            activeTab->Tick();
    }

    void CloseApp(wxCommandEvent & event)
    {
        myUpdateTimer->Stop();
        myApp->ExitMainLoop();
        event.Skip(); // Default handler will close window.
    }

    void OnClose(wxCloseEvent& event)
    {
        myUpdateTimer->Stop();
        myApp->ExitMainLoop();
        event.Skip(); // Default handler will close window.
    }

    void OnToggleToolbar(wxCommandEvent& WXUNUSED(event))
    {
        wxToolBar *tbar = GetToolBar();

        if ( !tbar )
        {
            RecreateToolbar();
        }
        else
        {
            // notice that there is no need to call SetToolBar(nullptr) here (although
            // this it is harmless to do and it must be called if you do not delete
            // the toolbar but keep it for later reuse), just delete the toolbar
            // directly and it will reset the associated frame toolbar pointer
            delete tbar;
        }
    }

    void RecreateToolbar()
    {
        // delete and recreate the toolbar
        wxToolBarBase *toolBar = GetToolBar();
        long style = toolBar ? toolBar->GetWindowStyle() : wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT;

        delete toolBar;

        SetToolBar(nullptr);

        style &= ~(wxTB_HORIZONTAL | wxTB_VERTICAL | wxTB_BOTTOM | wxTB_RIGHT | wxTB_HORZ_LAYOUT);
        style |= wxTB_TOP | wxTB_HORZ_TEXT;

        toolBar = CreateToolBar(style, wxID_HIGHEST);
        auto activeTab = GetActiveTab();
        if (activeTab)
        {
            activeTab->PopulateToolbar(toolBar);
        }

    }


    DECLARE_EVENT_TABLE()

private:
    void UpdatePages()
    {
        // Delete all pages except "About" and "Log" at the end.
        while (myTabs->GetPageCount() > 2)
        {
            myTabList.erase(myTabList.begin());
            myTabs->DeletePage(0);
        }

        auto pad = Device::Pad();
        if (pad)
        {
            AddTab(0, new SensitivityTab(myTabs, pad), SensitivityTab::Title, true);
            AddTab(1, new GraphTab(myTabs, pad), GraphTab::Title, true);
            AddTab(2, new MappingTab(myTabs, pad), MappingTab::Title);
            AddTab(3, new DeviceTab(myTabs), DeviceTab::Title);
            auto lights = Device::Lights();
            if (pad->featureLights && lights)
            {
                AddTab(4, new LightsTab(myTabs, lights), LightsTab::Title);
            }
        }
        else
        {
            AddTab(0, new IdleTab(myTabs), IdleTab::Title, true);
        }
    }

    void UpdateStatusText()
    {
        auto pad = Device::Pad();
        if (pad)
            SetStatusText(L"Connected to: " + pad->name, 0);
        else
            SetStatusText(wxEmptyString, 0);
    }

    void UpdatePollingRate()
    {
        auto rate = Device::PollingRate();
        if (rate > 0)
            SetStatusText(wxString::Format("%iHz", rate), 1);
        else
            SetStatusText(wxEmptyString, 1);
    }

    void AddTab(int index, BaseTab* tab, const wchar_t* title, bool select = false)
    {
        myTabs->InsertPage(index, tab->GetWindow(), title, select);
        myTabList.insert(myTabList.begin() + index, tab);
    }

    BaseTab* GetActiveTab()
    {
        auto page = myTabs->GetCurrentPage();
        for (auto tab : myTabList)
        {
            if (tab->GetWindow() == page)
                return tab;
        }
        return nullptr;
    }

    struct UpdateTimer : public wxTimer
    {
        UpdateTimer(MainWindow* owner) : owner(owner) {}
        void Notify() override { owner->Tick(); }
        MainWindow* owner;
    };

    wxString lastProfile = "";
    wxApp* myApp;
    wxNotebook* myTabs;
    vector<BaseTab*> myTabList;
    unique_ptr<wxTimer> myUpdateTimer;
};

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_CLOSE(MainWindow::OnClose)
    EVT_MENU(MENU_EXIT, MainWindow::CloseApp)
    EVT_MENU(PROFILE_LOAD, MainWindow::ProfileLoad)
    EVT_MENU(PROFILE_SAVE, MainWindow::ProfileSave)
    EVT_MENU(IDM_TOOLBAR_TOGGLE_TOOLBAR, MainWindow::OnToggleToolbar)
END_EVENT_TABLE()


// ====================================================================================================================
// Application.
// ====================================================================================================================

Application::~Application()
{
    wxString tempDir = GetTempDir(false);
    if (wxDirExists(tempDir)) {
        wxRmDir(tempDir);
    }

    if (doRestart) {
        wxExecute(argv[0]);
    }
}

wxString Application::GetTempDir()
{
    return GetTempDir(true);
}

wxString Application::GetTempDir(bool create)
{
    wxString dir = wxFileName::GetTempDir().Append(TOOL_NAME);
    if (!wxDirExists(dir) && create) {
        wxMkdir(dir);
    }
    return dir;
}

bool Application::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    Log::Init();

    auto versionString = wstring(TOOL_NAME) + L" " +
        to_wstring(ADP_VERSION_MAJOR) + L"." + to_wstring(ADP_VERSION_MINOR);

    auto now = wxDateTime::Now().FormatISOCombined(' ');
    Log::Writef(L"Application started: %ls - %ls", versionString.data(), now.wc_str());

    wxConfig * config = new wxConfig(TOOL_NAME);
    wxConfigBase::Set(config);

    //Updater::Init();
    Assets::Init();
    Device::Init();

    wxImage::AddHandler(new wxPNGHandler());

    wxIconBundle icons;

    auto icon16 = Files::Icon16();
    icons.AddIcon(icon16, wxBITMAP_TYPE_PNG);

    auto icon32 = Files::Icon32();
    icons.AddIcon(icon32, wxBITMAP_TYPE_PNG);

    auto icon64 = Files::Icon64();
    icons.AddIcon(icon64, wxBITMAP_TYPE_PNG);

    myWindow = new MainWindow(this, versionString.data());
    myWindow->SetIcons(icons);
    SettingsManager::Get().RestoreWindowGeometry(myWindow);
    myWindow->Show();

    /*
    Updater::CheckForAdpUpdates([](SoftwareUpdate& update) {
        ShowUpdateDialog(update);
    });
    */

    return true;
}

void Application::Restart()
{
    doRestart = true;
    ExitMainLoop();
}

int Application::OnExit()
{
    SettingsManager::Get().SaveWindowGeometry(myWindow);
    //Updater::Shutdown();
    Device::Shutdown();
    Assets::Shutdown();
    Log::Shutdown();
 
    delete wxConfigBase::Set(nullptr);
    return wxApp::OnExit();
}

}; // namespace adp.

wxIMPLEMENT_APP(adp::Application);
