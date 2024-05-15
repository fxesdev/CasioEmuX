#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class ExternalInterrupts : public Peripheral
	{
		MMURegion region_EXICON, region_F048, region_F049, region_F04A, region_F04B, region_F04C;
        uint8_t data_F048, data_F049, data_F04A, data_F04B, data_F04C;

        bool pin_level_1, pin_level_2, pin_level_3;

        size_t EXI1INT = 1;
        size_t EXI2INT = 2;
        size_t EXI3INT = 3;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Reset();
		void Tick();
	};
}
