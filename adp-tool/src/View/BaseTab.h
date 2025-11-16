#pragma once

#include "wx/window.h"
#include "wx/toolbar.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Model/Device.h"

namespace adp {

class BaseTab
{
public:
    virtual ~BaseTab() {}

    virtual wxWindow* GetWindow() = 0;

    virtual void HandleChanges(DeviceChanges changes) {}

    virtual void PopulateToolbar(wxToolBarBase* toolBar) {}

    virtual void Tick() {}
};

}; // namespace adp.
