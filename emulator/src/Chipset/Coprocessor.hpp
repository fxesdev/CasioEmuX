#pragma once
#include "../Config.hpp"

#include <cstdint>

namespace casioemu {
	class Emulator;

	enum CoproFlags {
		COPRO_Q = 0b00001000,
		COPRO_OV = 0b00010000,
		COPRO_S = 0b00100000,
		COPRO_Z = 0b01000000,
		COPRO_C = 0b10000000
	};

	class Coprocessor {
		Emulator& emulator;

		bool start_operation;
		int tick_counter;

		uint8_t backup_CR[16];

		const uint8_t copro_id = 0x81;

	public:
		Coprocessor(Emulator& _emulator);

		void Mul16();
		void Div16();
		void MAC(bool sat);
		void Div32();

		void Tick();
		void Refresh();
		void Reset();
		void SetCR(int index, uint8_t data);
	};
}