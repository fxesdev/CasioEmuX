#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class Miscellaneous : public Peripheral
	{
		MMURegion region_dsr, region_F004, region_F048, region_F220;
		uint64_t data_F048;
		uint32_t data_F220;

		static constexpr uint16_t addr [] = {
			0xF033, 0xF034, // both HW_ES_PLUS, HW_CLASSWIZ and HW_CLASSWIZ_II
			0xF035, 0xF036, 0xF039, 0xF03D, 0xF224// HW_CLASSWIZ and HW_CLASSWIZ_II
		};
		static constexpr int N_BYTE = sizeof(addr) / sizeof(addr[0]);
		MMURegion region [N_BYTE];
		uint8_t data [N_BYTE];

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Tick();
		void Reset();
	};
}

