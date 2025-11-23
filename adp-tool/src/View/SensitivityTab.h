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

    float myReleaseThreshold = -1.f;
    int myAdjustingSensorIndex;
    double myAdjustingSensorThreshold;
};

}; // namespace adp.
