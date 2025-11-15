#pragma once

#include "wx/window.h"
#include "wx/sizer.h"
#include "wx/slider.h"

#include "View/BaseTab.h"

#include <vector>

using namespace std;

namespace adp {

class GraphDisplay;

class GraphTab : public BaseTab, public wxWindow
{
public:
    static const wchar_t* Title;

    GraphTab(wxWindow* owner, const PadState* pad);

    void HandleChanges(DeviceChanges changes) override;
    void Tick() override;

    wxWindow* GetWindow() override { return this; }

private:
    void UpdateDisplays();

    vector<GraphDisplay*> myGraphDisplays;
    wxBoxSizer* mySensorSizer;
    double myReleaseThreshold = 1.0;
    bool myIsAdjustingReleaseThreshold = false;
};

}; // namespace adp.
