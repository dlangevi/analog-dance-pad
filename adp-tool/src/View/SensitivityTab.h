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
    int myAdjustingSensorIndex;
    double myAdjustingSensorThreshold;
    double myAdjustingSensorReleaseOffset;
};

}; // namespace adp.
