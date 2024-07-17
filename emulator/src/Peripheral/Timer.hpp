#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class Timer : public Peripheral
	{
		MMURegion region_counter, region_interval, region_F024, region_control;
		uint16_t data_counter, data_interval;
		uint8_t data_F024, data_control;

		size_t TM0INT = 4;

		uint64_t ext_to_int_counter, ext_to_int_next, ext_to_int_int_done;

		size_t TimerFreqDiv;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Reset();
		void Tick();
		void Uninitialise();
	};
}

