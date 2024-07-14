#include "Coprocessor.hpp"

#include "CPU.hpp"
#include "../Emulator.hpp"
#include "Chipset.hpp"

namespace casioemu {
	Coprocessor::Coprocessor(Emulator& _emulator) : emulator(_emulator) {
	}

	void Coprocessor::Tick() {

	}

	void Coprocessor::Reset() {
		for (int i = 0; i < 15; i++)
			emulator.chipset.cpu.reg_cr[i] = 0;

		emulator.chipset.cpu.reg_cr[15] = copro_id;
	}

	void Coprocessor::SetCR(int index, uint8_t data) {
		if (index < 8) {
			emulator.chipset.cpu.reg_cr[index] = data;
		}
		else if (index == 8) {
			emulator.chipset.cpu.reg_cr[8] = data & 0x97;
		}
		else if (index == 9) {
			emulator.chipset.cpu.reg_cr[9] &= 0x01;
			emulator.chipset.cpu.reg_cr[9] |= data & 0xF8;
		}
	}
}