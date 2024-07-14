#pragma once
#include "../Config.hpp"

#include <cstdint>

namespace casioemu {
	class Emulator;

	class Coprocessor {
		Emulator& emulator;

		const uint8_t copro_id = 0x81;

	public:
		Coprocessor(Emulator& _emulator);

		void Tick();
		void Reset();
		void SetCR(int index, uint8_t data);
	};
}