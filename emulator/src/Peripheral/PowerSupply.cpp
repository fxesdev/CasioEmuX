#include "PowerSupply.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

#include <cmath>

namespace casioemu {
    void PowerSupply::Initialise() {
        clock_type = CLOCK_UNDEFINED;
        
        BLDMode = 0;
        BLDControl = 0;
        threshold = 0;
        data_BLDCON2 = 0;
        data_SPIndicator = 0;
        isTestRoutineRunning = false;

        BLDFlag = emulator.BatteryVoltage >= ThreshVoltage[0] ? 1 : 0;

        region_BLDCON0.Setup(0xF0D0, 1, "BatteryLevelDetector/BLDCON0", &threshold, MMURegion::DefaultRead<uint8_t, 0x1F>, MMURegion::DefaultWrite<uint8_t, 0x1F>, emulator);
        region_BLDCON1.Setup(0xF0D1, 1, "BatteryLevelDetector/BLDCON1", this, [](MMURegion *region, size_t) {
            PowerSupply *powersupply = (PowerSupply*)region->userdata;
            uint8_t ReadData = (powersupply->BLDMode << 2) | (powersupply->BLDFlag << 1) | powersupply->BLDControl;
            return ReadData;
        }, [](MMURegion *region, size_t, uint8_t data) {
            PowerSupply *powersupply = (PowerSupply*)region->userdata;
            powersupply->BLDMode = (data & 0x04) >> 2;
            if(data & 0x01) {
                powersupply->BLDControl = 1;
                powersupply->StartTest(powersupply->BLDMode, false, powersupply->ThreshVoltage[powersupply->threshold]);
            } else {
                powersupply->BLDControl = 0;
                powersupply->isTestRoutineRunning = false;
            }
        }, emulator);
        region_BLDCON2.Setup(0xF0D2, 1, "BatteryLevelDetector/BLDCON2", &data_BLDCON2, MMURegion::DefaultRead<uint8_t, 0x37>, MMURegion::DefaultWrite<uint8_t, 0x37>, emulator);

        region_SPIndicator.Setup(0xF310, 1, "SolarPanelIndicator/0xF310*1", this, [](MMURegion *region, size_t) {
            PowerSupply *powersupply = (PowerSupply*)region->userdata;
            bool isSPAvailable = powersupply->emulator.SolarPanelVoltage >= powersupply->SolarPanelThresh ? 1 : 0;
            return (uint8_t)((powersupply->data_SPIndicator & 0x0F) | (isSPAvailable << 4));
        }, [](MMURegion *region, size_t, uint8_t data) {
            PowerSupply *powersupply = (PowerSupply*)region->userdata;
            powersupply->data_SPIndicator = (data & 0x0F);
        }, emulator);

        if(emulator.hardware_id == HW_FX_5800P) {
            region_5800P_BLD.Setup(0x100000, 1, "PowerSupply/Fx5800P_BLD", &emulator.BatteryVoltage, [](MMURegion* region, size_t) {
                return (uint8_t)(*(float*)region->userdata >= 1.0f ? 1 : 0);
            }, MMURegion::IgnoreWrite, emulator);
        }

        *(PowerSupply **)lua_newuserdata(emulator.lua_state, sizeof(PowerSupply *)) = this;
        lua_newtable(emulator.lua_state);
        lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			PowerSupply *powersupply = *(PowerSupply **)lua_topointer(lua_state, 1);
            std::string index = lua_tostring(lua_state, 2);
            if(index == "bt") {
                lua_pushnumber(lua_state, powersupply->emulator.BatteryVoltage);
                return 1;
            }
            if(index == "sp") {
                lua_pushnumber(lua_state, powersupply->emulator.SolarPanelVoltage);
                return 1;
            }
			return 0;
		});
        lua_setfield(emulator.lua_state, -2, "__index");
        lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			PowerSupply *powersupply = *(PowerSupply **)lua_topointer(lua_state, 1);
            std::string index = lua_tostring(lua_state, 2);
            if(index == "bt") {
                powersupply->emulator.BatteryVoltage = lua_tonumber(lua_state, 3);
                return 0;
            }
            if(index == "sp") {
                powersupply->emulator.SolarPanelVoltage = lua_tonumber(lua_state, 3);
                return 0;
            }
			return 0;
		});
        lua_setfield(emulator.lua_state, -2, "__newindex");
        lua_setmetatable(emulator.lua_state, -2);
        lua_setglobal(emulator.lua_state, "power");
    }

    void PowerSupply::StartTest(bool TestMode, bool AutoRep, float thresh) {
        if(isTestRoutineRunning)
            return;
        
        isTestRoutineRunning = true;
        CurrentTestMode = TestMode;
        CurrentRepMode = AutoRep;
        CurrentThresh = thresh;
        HasResult = false;
        TestTimer = 0;
    }

    void PowerSupply::TestTick() {
        TestTimer++;
        if(TestTimer >= TestRoutineTicks) {
            StopTest();
            return;
        }
        if(TestTimer < InitTicks || HasResult)
            return;
        
        float BatteryVoltage = emulator.BatteryVoltage;
        if(BatteryVoltage < CurrentThresh * (1 - StandardError)) {
            BLDFlag = 0;
            HasResult = true;
            if(data_BLDCON2 & 0x20)
                emulator.chipset.MaskableInterrupts[BLOWINT].TryRaise();
            return;
        }
        if(BatteryVoltage > CurrentThresh * (1 + StandardError)) {
            BLDFlag = 1;
            HasResult = true;
            return;
        }
        float RelVolt = (BatteryVoltage - CurrentThresh) / StandardError;
        if(CurrentTestMode) {
            if(TestTimer >= (0.5-RelVolt/2)*(TestRoutineTicks-InitTicks)+InitTicks) {
                BLDFlag = 1;
                HasResult = true;
            }
        } else {
            if(RelVolt > 0) {
                if(TestTimer >= (0.5-RelVolt/2)*(TestRoutineTicks-InitTicks)+InitTicks) {
                    BLDFlag = 1;
                    HasResult = true;
                }
            } else {
                if(TestTimer >= (0.5+RelVolt/2)*(TestRoutineTicks-InitTicks)+InitTicks) {
                    BLDFlag = 0;
                    HasResult = true;
                    if(data_BLDCON2 & 0x20)
                        emulator.chipset.MaskableInterrupts[BLOWINT].TryRaise();
                    return;
                }
            }
        }
    }

    void PowerSupply::StopTest() {
        if(!isTestRoutineRunning)
            return;
        
        isTestRoutineRunning = false;
        if(!(data_BLDCON2 & 0x07)) {
            if(data_BLDCON2 & 0x10)
                emulator.chipset.MaskableInterrupts[BENDINT].TryRaise();
            BLDControl = 0;
            return;
        }
        if(CurrentRepMode) {
            StartTest(BLDMode, (data_BLDCON2 & 0x07) == 0x07 ? true : false, ThreshVoltage[threshold]);
            return;
        }
        if(data_BLDCON2 & 0x10)
            emulator.chipset.MaskableInterrupts[BENDINT].TryRaise();
        TestTimer = 0;
        DelayTicks = std::pow(2, -(data_BLDCON2 & 0x07)) * emulator.GetCyclesPerSecond();
    }

    void PowerSupply::Tick() {
        if(isTestRoutineRunning) {
            TestTick();
        } else if(BLDControl) {
            TestTimer++;
            if(TestTimer >= DelayTicks)
                StartTest(BLDMode, (data_BLDCON2 & 0x07) == 0x07 ? true : false, ThreshVoltage[threshold]);
        }
    }

    void PowerSupply::Reset() {
        BLDMode = 0;
        BLDControl = 0;
        threshold = 0;
        data_BLDCON2 = 0;
        data_SPIndicator = 0;
        isTestRoutineRunning = false;
        BLDFlag = emulator.BatteryVoltage >= ThreshVoltage[0] ? 1 : 0;
    }
}