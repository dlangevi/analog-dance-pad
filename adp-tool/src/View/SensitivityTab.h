#pragma once

#include <Adp.h>

namespace adp {

class SensitivityTab
{
public:
    SensitivityTab();
    void Render();

private:
    void RenderSensor(int);
    // -1.0 for unset
    float myReleaseThreshold = -1.0f;
    int myAdjustingSensorIndex;
    double myAdjustingSensorThreshold;
};

}; // namespace adp.
