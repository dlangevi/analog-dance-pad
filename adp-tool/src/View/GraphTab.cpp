#include "Adp.h"

#include <deque>
#include <wx/memory.h>

#include "wx/dcbuffer.h"
#include "wx/stattext.h"
#include "wx/timer.h"

#include "Assets/Assets.h"

#include "View/GraphTab.h"

using namespace std;

namespace adp {

static constexpr int SENSOR_INDEX_NONE = -1;

class GraphDisplay : public wxWindow
{
public:
    GraphDisplay(GraphTab* owner)
        : wxWindow(owner, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBG_STYLE_PAINT | wxFULL_REPAINT_ON_RESIZE)
        , myOwner(owner)
    {
        SetMinSize(wxSize(10, 50)); 
        myUpdateTimer.Bind(wxEVT_TIMER, &GraphDisplay::OnTimer, this);
        myUpdateTimer.Start(16); // ~60 FPS, adjust as needed
        
    }

    void Tick()
    {
        if (myAdjustingSensorIndex != SENSOR_INDEX_NONE)
        {
            auto mouse = wxGetMouseState();
            auto rect = GetScreenRect();
            double value = double(mouse.GetY() - rect.y);
            double range = max(1.0, (double)rect.height);
            myAdjustingSensorThreshold = clamp(1.0 - (value / range), 0.0, 1.0);
            if (!mouse.LeftIsDown())
            {
                Device::SetThreshold(myAdjustingSensorIndex, myAdjustingSensorThreshold);
                myAdjustingSensorIndex = SENSOR_INDEX_NONE;
            }
        }
    }

    void OnPaint(wxPaintEvent& evt)
    {
        wxBufferedPaintDC dc(this);
        auto size = GetClientSize();
        
        // Ensure history is initialized
        if (mySensorHistory.size() != mySensorIndices.size()) {
            mySensorHistory.resize(mySensorIndices.size());
        }
        
        int graphHeight = size.y / mySensorIndices.size();
        
        for (size_t i = 0; i < mySensorIndices.size(); ++i)
        {
            int y = i * graphHeight;
            auto sensor = Device::Sensor(mySensorIndices[i]);
            auto pressed = sensor ? sensor->pressed : false;
            auto threshold = sensor ? sensor->threshold : 0.0;
            
            if (myAdjustingSensorIndex == mySensorIndices[i])
                threshold = myAdjustingSensorThreshold;
            
            int thresholdY = y + graphHeight - (threshold * graphHeight);
            
            // Background
            dc.SetPen(Pens::Black1px());
            dc.SetBrush(Brushes::SensorBar());
            dc.DrawRectangle(0, y, size.x, graphHeight);
            
            // Draw threshold line
            dc.SetPen(wxPen(*wxGREEN, 2));
            dc.DrawLine(0, thresholdY, size.x, thresholdY);
            
            // Draw the graph line
            auto& history = mySensorHistory[i];
            if (history.values.size() > 1)
            {
                dc.SetPen(wxPen(pressed ? *wxRED : *wxBLUE, 2));
                
                float xStep = size.x / (float)SensorHistory::MAX_SAMPLES;
                int startIdx = 0;
                
                for (size_t j = 1; j < history.values.size(); ++j)
                {
                    int x1 = (startIdx + j - 1) * xStep;
                    int x2 = (startIdx + j) * xStep;

                    if (history.values[j - 1] > threshold)
                    {
                      dc.SetPen(wxPen(*wxRED, 2));
                    }
                    else
                    {
                      dc.SetPen(wxPen(*wxBLUE, 2));
                    }
                    
                    int value1Y = y + graphHeight - (history.values[j - 1] * graphHeight);
                    int value2Y = y + graphHeight - (history.values[j] * graphHeight);
                    
                    dc.DrawLine(x1, value1Y, x2, value2Y);
                }
                
                // Optional: Fill area under the curve
                if (history.values.size() > 0)
                {
                    dc.SetBrush(wxBrush(pressed ? wxColour(255, 0, 0, 50) : wxColour(0, 0, 255, 50)));
                    dc.SetPen(*wxTRANSPARENT_PEN);
                    
                    wxPoint* points = new wxPoint[history.values.size() + 2];
                    
                    for (size_t j = 0; j < history.values.size(); ++j)
                    {
                        int x = (startIdx + j) * xStep;
                        int valueY = y + graphHeight - (history.values[j] * graphHeight);
                        points[j] = wxPoint(x, valueY);
                    }
                    
                    // Close the polygon at the bottom
                    points[history.values.size()] = wxPoint((startIdx + history.values.size() - 1) * xStep, y + graphHeight);
                    points[history.values.size() + 1] = wxPoint(0, y + graphHeight);
                    
                    dc.DrawPolygon(history.values.size() + 2, points);
                    delete[] points;
                }
            }
            
            // Draw current value indicator on the right edge
            if (sensor)
            {
                int currentY = y + graphHeight - (sensor->value * graphHeight);
                dc.SetBrush(pressed ? Brushes::SensorOn() : Brushes::SensorOff());
                dc.SetPen(Pens::Black1px());
                dc.DrawCircle(size.x - 10, currentY, 5);
            }
            
            // Draw sensitivity text
            auto sensitivityText = wxString::Format("%i%%", (int)std::lround(threshold * 100.0));
            auto rect = wxRect(5, y + 5, 120, 20);
            dc.SetBrush(Brushes::DarkGray());
            dc.DrawRectangle(rect);
            dc.SetTextForeground(*wxWHITE);
            dc.DrawLabel(sensitivityText, rect, wxALIGN_CENTER);
            
            // Draw separator line between graphs
            if (i < mySensorIndices.size() - 1)
            {
                dc.SetPen(wxPen(*wxBLACK, 1));
                dc.DrawLine(0, y + graphHeight, size.x, y + graphHeight);
            }
        }
    }

    void OnClick(wxMouseEvent& event)
    {
        auto pos = event.GetPosition();
        auto rect = GetClientRect();
        if (rect.Contains(pos))
        {
            int barIndex = (pos.x - rect.x) * mySensorIndices.size() / max(1, rect.width);
            if (barIndex >= 0 && barIndex < (int)mySensorIndices.size())
                myAdjustingSensorIndex = mySensorIndices[barIndex];
        }
    }

    void SetTarget(int button, const vector<int>& sensorIndices)
    {
        myButton = button;
        mySensorIndices = sensorIndices;
    }

    int button() const { return myButton; }

    DECLARE_EVENT_TABLE()

protected:
    wxSize DoGetBestSize() const override { return wxSize(20, 100); }

private:
    GraphTab* myOwner;
    int myButton = 0;
    vector<int> mySensorIndices;
    int myAdjustingSensorIndex = SENSOR_INDEX_NONE;
    double myAdjustingSensorThreshold = 0.0;

    struct SensorHistory {
        std::deque<float> values;
        static constexpr size_t MAX_SAMPLES = 200; // Adjust based on desired time window
        
        void AddValue(float value) {
            values.push_back(value);
            if (values.size() > MAX_SAMPLES) {
                values.pop_front();
            }
        }
        
        void Clear() {
            values.clear();
        }
    };
    
    std::vector<SensorHistory> mySensorHistory;
    wxTimer myUpdateTimer;
    
    void OnTimer(wxTimerEvent& evt) {
        // Update sensor history
        for (size_t i = 0; i < mySensorIndices.size(); ++i) {
            auto sensor = Device::Sensor(mySensorIndices[i]);
            float value = sensor ? sensor->value : 0.0f;
            
            if (i >= mySensorHistory.size()) {
                mySensorHistory.resize(mySensorIndices.size());
            }
            mySensorHistory[i].AddValue(value);
        }
        Refresh();
    }

};

BEGIN_EVENT_TABLE(GraphDisplay, wxWindow)
    EVT_PAINT(GraphDisplay::OnPaint)
    EVT_LEFT_DOWN(GraphDisplay::OnClick)
END_EVENT_TABLE()

const wchar_t* GraphTab::Title = L"Graph";

GraphTab::GraphTab(wxWindow* owner, const PadState* pad)
    : wxWindow(owner, wxID_ANY)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    mySensorSizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(mySensorSizer, 1, wxEXPAND);
    UpdateDisplays();

    SetSizer(sizer);

    myIsAdjustingReleaseThreshold = false;
}

void GraphTab::HandleChanges(DeviceChanges changes)
{
    if (changes & DCF_BUTTON_MAPPING)
        UpdateDisplays();
}

void GraphTab::Tick()
{
    for (auto display : myGraphDisplays)
    {
        display->Tick();
        display->Refresh(false);
    }
}

void GraphTab::UpdateDisplays()
{
    map<int, vector<int>> mapping; // button -> sensors[]

    auto pad = Device::Pad();
    if (pad)
    {
        for (int i = 0; i < pad->numSensors; ++i)
        {
            auto sensor = Device::Sensor(i);
            if (sensor->button != 0)
                mapping[sensor->button].push_back(i);
        }
    }

    if (mapping.size() != myGraphDisplays.size())
    {
        mySensorSizer->Clear(true);
        myGraphDisplays.clear();
        for (auto mapping : mapping)
        {
            auto display = new GraphDisplay(this);
            myGraphDisplays.push_back(display);
            mySensorSizer->Add(display, 1, wxALL | wxEXPAND, 4);
        }
        mySensorSizer->Layout();
    }
    
    int index = 0;
    for (auto mapping : mapping)
        myGraphDisplays[index++]->SetTarget(mapping.first, mapping.second);
}

}; // namespace adp.
