#pragma once

#include "wx/window.h"
#include "wx/colordlg.h"
#include "wx/dcbuffer.h"
#include "wx/toolbar.h"
#include "wx/sizer.h"

#include "Assets/Assets.h"
#include "Model/Device.h"
#include "Model/SettingsManager.h"


using namespace std;

namespace adp {

wxDEFINE_EVENT(EVT_COLOR_CHANGED, wxCommandEvent);

static wxColour ToWx(RgbColor color)
{
    return wxColour(color.red, color.green, color.blue);
}

class ColorSetting: public wxWindow
{
public:
    ColorSetting(
        wxWindow* owner,
        wxPoint position,
        wxSize size,
        wxString tab,
        wxString label,
        wxColour* color
        )
        : wxWindow(owner, wxID_ANY, position, size)
        , myTab(tab)
        , myLabel(label)
        , myColor(color)
    {

        wxString tabLabel = myTab + "/" + myLabel;
        *myColor = SettingsManager::Get().GetColor(tabLabel, *color);
    }

    void EnableFade(bool enabled)
    {
        myIsFadeEnabled = enabled;
        Refresh();
    }

    bool IsFadeEnabled()
    {
        return myIsFadeEnabled;
    }

    wxColour GetColor()
    {
        return *myColor;
    }

    void OnPaint(wxPaintEvent& evt)
    {
        auto size = GetClientSize();
        wxBufferedPaintDC dc(this);

        auto startColor = *myColor;
        auto centerColor = startColor;

        // Start color (end color is not used).
        auto rect = wxRect(0, 0, size.x, size.y);
        dc.SetPen(Pens::Black1px());
        dc.DrawRectangle(rect);
        dc.SetPen(Pens::White1px());
        dc.SetBrush(wxBrush(startColor, wxBRUSHSTYLE_SOLID));
        rect.Deflate(1);
        dc.DrawRectangle(rect);

        // Label text.
        auto rec2t = wxRect(size.x / 2 - 20, 5, 40, 20);
        auto luminance = (0.299*centerColor.Red() + 0.587*centerColor.Green() + 0.114*centerColor.Blue()) / 255.0;
        bool isBright = luminance > 0.5;
        dc.SetTextForeground(isBright ? *wxBLACK : *wxWHITE);
        dc.DrawLabel(myLabel, rec2t, wxALIGN_CENTER);
    }

    void OnClick(wxMouseEvent& event)
    {
        auto pos = event.GetPosition();
        auto rect = GetClientRect();
        if (rect.Contains(pos))
        {
            ShowPicker(myColor);
        }
    }

    bool ShowPicker(wxColour* color)
    {
        wxColourData data;
        data.SetColour(*color);
        wxColourDialog dlg(this, &data);
        if (dlg.ShowModal() != wxID_OK)
            return false;

        auto result = dlg.GetColourData().GetColour();
        Refresh();
        *color = result;

        // Set the color based on the tab and label.
        wxString tabLabel = myTab + "/" + myLabel;
        SettingsManager::Get().SetColor(tabLabel, *color);

        // wxCommandEvent event(EVT_COLOR_CHANGED);
        //
        // wxPostEvent(this, event);

        return true;
    }

    DECLARE_EVENT_TABLE()

private:
    bool myIsFadeEnabled;
    wxColour* myColor;
    wxString myTab;
    wxString myLabel;
};

BEGIN_EVENT_TABLE(ColorSetting, wxWindow)
    EVT_PAINT(ColorSetting::OnPaint)
    EVT_LEFT_DOWN(ColorSetting::OnClick)
END_EVENT_TABLE()


static ColorSetting * CreateColorSetting(
    wxToolBarBase* toolBar,
    wxString tab,
    wxString label,
    wxColour* color)
{

    wxControl* colorControl = new wxControl(toolBar, wxID_ANY);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    auto colourPicker = new ColorSetting(colorControl, wxPoint(0, 0), wxSize(140, 30), tab, label, color);

    sizer->Add(colourPicker, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 4);

    colorControl->SetSizer(sizer);
    toolBar->AddControl(colorControl);
    return colourPicker;
}

} // namespace adp
