#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu {
    class PowerSupply : public Peripheral {
        MMURegion region_BLDCON0, region_BLDCON1, region_BLDCON2, region_SPIndicator;
        uint8_t threshold, data_BLDCON2, data_SPIndicator;
        bool BLDMode, BLDFlag, BLDControl;

        MMURegion region_5800P_BLD;
        
        bool isTestRoutineRunning;
        size_t TestTimer;
        bool CurrentTestMode, CurrentRepMode, HasResult;
        float CurrentThresh;

        size_t DelayTicks;

        size_t BENDINT = 12;
        size_t BLOWINT = 13;

        //Currently most values for BatteryLevelDetector are just simulation and not tested on real hardware.
        float ThreshVoltage[32] = {1.10, 1.15, 1.20, 1.25, 1.30, 1.35, 1.40, 1.45, 1.50, 1.60, 1.70, 1.80, 1.90, 2.00, 2.10, 2.20,
                                   2.30, 2.40, 2.50, 2.60, 2.70, 2.80, 2.90, 3.00, 3.10, 3.20, 3.30, 3.40, 3.50, 3.60, 3.80, 4.00};
        const float StandardError = 0.02;

        const float SolarPanelThresh = 1.0;

        const size_t TestRoutineTicks = 131072;
        const size_t InitTicks = 16384;

    public:
        using Peripheral::Peripheral;

        void StartTest(bool TestMode, bool AutoRep, float thresh);
        void TestTick();
        void StopTest();

        void Initialise();
        void Tick();
        void Reset();
    };
}