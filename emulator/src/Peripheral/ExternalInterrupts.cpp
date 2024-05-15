#include "ExternalInterrupts.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
    void ExternalInterrupts::Initialise() {
        clock_type = CLOCK_UNDEFINED;

        emulator.chipset.data_EXICON = 0;

        region_EXICON.Setup(0xF018, 1, "ExternalInterrupts/EXICON", &emulator.chipset.data_EXICON, MMURegion::DefaultRead<uint8_t>, MMURegion::DefaultWrite<uint8_t>, emulator);
    }

    void ExternalInterrupts::Tick() {
        //EXI0INT is handled by keyboard
        
    }

    void ExternalInterrupts::Reset() {
        emulator.chipset.data_EXICON = 0;
    }
}