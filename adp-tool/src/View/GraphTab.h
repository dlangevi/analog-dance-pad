#pragma once

#include <Adp.h>

#include <View/Colors.h>
#include <View/GraphHistory.h>

namespace adp {

class GraphTab 
{
public:
    GraphTab();
    void Render();

private:
    void RenderSensor(int, RgbColorf&, GraphHistory&);
    int myAdjustingSensorIndex;
    double myAdjustingSensorThreshold;
    bool myCheckBox;
    vector<GraphHistory> myGraphHistories;
};

}; // namespace adp.
