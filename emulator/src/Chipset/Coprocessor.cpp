#include "Coprocessor.hpp"

#include "CPU.hpp"
#include "../Emulator.hpp"
#include "Chipset.hpp"

namespace casioemu {
	Coprocessor::Coprocessor(Emulator& _emulator) : emulator(_emulator) {
	}

	void Coprocessor::Mul16() {
		uint16_t op0 = backup_CR[4] | (backup_CR[5] << 8);
		uint16_t op1 = backup_CR[6] | (backup_CR[7] << 8);
		uint32_t res = 0;
		if (!op0 || !op1) {
			backup_CR[9] |= COPRO_Z;
		}
		else {
			backup_CR[9] &= ~COPRO_Z;
			if (backup_CR[8] & 0x10) {
				bool sign0 = false;
				bool sign1 = false;
				if (op0 & 0x8000) {
					sign0 = true;
					op0 = (~op0 + 1) & 0xFFFF;
				}
				if (op1 & 0x8000) {
					sign1 = true;
					op1 = (~op1 + 1) & 0xFFFF;
				}
				res = (op0 * op1) & 0x7FFFFFFF;
				if (sign0 ^ sign1)
				{
					res = ~res + 1;
					backup_CR[9] |= COPRO_S;
				}
				else {
					backup_CR[9] &= ~COPRO_S;
				}
			}
			else {
				res = (op0 * op1) & 0xFFFFFFFF;
			}
		}
		
		for (int i = 0; i < 4; i++)
			backup_CR[i] = (res >> (i * 8)) & 0xFF;
	}

	void Coprocessor::Div16() {
		uint32_t op0 = backup_CR[0] | (backup_CR[1] << 8) | (backup_CR[2] << 16) | (backup_CR[3] << 24);
		uint16_t op1 = backup_CR[6] | (backup_CR[7] << 8);
		uint32_t res = 0;
		uint16_t remain = 0;
		if (backup_CR[8] & 0x10) {
			if (!op1) {
				if (op0 & 0x80000000) {
					res = 0x80000000;
					remain = (op0 & 0x7FFF) | 0x8000;
					backup_CR[9] |= COPRO_S;
				}
				else {
					res = 0x7FFFFFFF;
					remain = op0 & 0x7FFF;
					backup_CR[9] &= ~COPRO_S;
				}
				backup_CR[9] |= COPRO_C | COPRO_Z;
			}
			else {
				bool sign0 = false;
				bool sign1 = false;
				if (op0 & 0x80000000) {
					sign0 = true;
					op0 = (~op0 + 1) & 0xFFFFFFFF;
				}
				if (op1 & 0x8000) {
					sign1 = true;
					op1 = (~op1 + 1) & 0xFFFF;
				}
				res = op0 / op1;
				remain = op0 % op1;
				if (sign0 ^ sign1)
				{
					res = ~res + 1;
					backup_CR[9] |= COPRO_S;
				}
				else {
					backup_CR[9] &= ~COPRO_S;
				}
				if (sign0)
					remain = ~remain + 1;
				if (!res)
					backup_CR[9] |= COPRO_Z;
				else
					backup_CR[9] &= ~COPRO_Z;
				backup_CR[9] &= ~COPRO_C;
			}
			backup_CR[9] &= ~COPRO_OV;
		}
		else {
			if (!op1) {
				res = 0xFFFFFFFF;
				remain = op0 & 0xFFFF;
				backup_CR[9] |= COPRO_C;
				backup_CR[9] &= ~COPRO_Z;
			}
			else {
				res = op0 / op1;
				remain = op0 % op1;
				if (!res)
					backup_CR[9] |= COPRO_Z;
				else
					backup_CR[9] &= ~COPRO_Z;
				backup_CR[9] &= ~COPRO_C;
			}
		}
		for (int i = 0; i < 4; i++)
			backup_CR[i] = (res >> (i * 8)) & 0xFF;
		backup_CR[4] = remain & 0xFF;
		backup_CR[5] = (remain >> 8) & 0xFF;
	}

	void Coprocessor::MAC(bool sat) {
		uint16_t op0 = backup_CR[4] | (backup_CR[5] << 8);
		uint16_t op1 = backup_CR[6] | (backup_CR[7] << 8);
		uint32_t op2 = backup_CR[0] | (backup_CR[1] << 8) | (backup_CR[2] << 16) | (backup_CR[3] << 24);
		uint32_t res = 0;
		if (backup_CR[8] & 0x10) {
			bool sign0 = false;
			bool sign1 = false;
			if (op0 & 0x8000) {
				sign0 = true;
				op0 = (~op0 + 1) & 0xFFFF;
			}
			if (op1 & 0x8000) {
				sign1 = true;
				op1 = (~op1 + 1) & 0xFFFF;
			}
			res = (op0 * op1) & 0x7FFFFFFF;
			if (sign0 ^ sign1)
				res = ~res + 1;
		}
		else {
			res = (op0 * op1) & 0xFFFFFFFF;
		}
		uint64_t op3 = res;
		res += op2;
		if (!res)
			backup_CR[9] |= COPRO_Z;
		else
			backup_CR[9] &= ~COPRO_Z;
		if (op3 + op2 > 0xFFFFFFFF)
			backup_CR[9] |= COPRO_C;
		else
			backup_CR[9] &= ~COPRO_C;
		if (backup_CR[8] & 0x10) {
			if (((op3 & op2) & 0x80000000) && !(res & 0x80000000)) {
				backup_CR[9] |= COPRO_OV;
				if (sat) {
					backup_CR[9] |= COPRO_Q;
					res = 0x80000000;
				}
			} else if (!((op3 | op2) & 0x80000000) && (res & 0x80000000)) {
				backup_CR[9] |= COPRO_OV;
				if (sat) {
					backup_CR[9] |= COPRO_Q;
					res = 0x7FFFFFFF;
				}
			} else {
				backup_CR[9] &= ~COPRO_OV;
			}
		} else {
			if (op3 + op2 > 0xFFFFFFFF) {
				backup_CR[9] |= COPRO_OV;
				if (sat) {
					backup_CR[9] |= COPRO_Q;
					res = 0xFFFFFFFF;
				}
			} else {
				backup_CR[9] &= ~COPRO_OV;
			}
		}
		if (res & 0x80000000)
			backup_CR[9] |= COPRO_S;
		else
			backup_CR[9] &= ~COPRO_S;
		for (int i = 0; i < 4; i++)
			backup_CR[i] = (res >> (i * 8)) & 0xFF;
	}

	void Coprocessor::Div32() {
		uint32_t op0 = backup_CR[0] | (backup_CR[1] << 8) | (backup_CR[2] << 16) | (backup_CR[3] << 24);
		uint32_t op1 = backup_CR[4] | (backup_CR[5] << 8) | (backup_CR[6] << 16) | (backup_CR[7] << 24);
		uint32_t res = 0;
		uint32_t remain = 0;
		if (backup_CR[8] & 0x10) {
			if (!op1) {
				if (op0 & 0x80000000) {
					res = 0x80000000;
					backup_CR[9] |= COPRO_S;
				}
				else {
					res = 0x7FFFFFFF;
					backup_CR[9] &= ~COPRO_S;
				}
				remain = op0;
				backup_CR[9] |= COPRO_C | COPRO_Z;
			}
			else {
				bool sign0 = false;
				bool sign1 = false;
				if (op0 & 0x80000000) {
					sign0 = true;
					op0 = (~op0 + 1) & 0xFFFFFFFF;
				}
				if (op1 & 0x80000000) {
					sign1 = true;
					op1 = (~op1 + 1) & 0xFFFFFFFF;
				}
				res = op0 / op1;
				remain = op0 % op1;
				if (sign0 ^ sign1)
				{
					res = ~res + 1;
					backup_CR[9] |= COPRO_S;
				}
				else {
					backup_CR[9] &= ~COPRO_S;
				}
				if (sign0)
					remain = ~remain + 1;
				if (!res)
					backup_CR[9] |= COPRO_Z;
				else
					backup_CR[9] &= ~COPRO_Z;
				backup_CR[9] &= ~COPRO_C;
			}
			backup_CR[9] &= ~COPRO_OV;
		}
		else {
			if (!op1) {
				res = 0xFFFFFFFF;
				remain = op0;
				backup_CR[9] |= COPRO_C;
				backup_CR[9] &= ~COPRO_Z;
			}
			else {
				res = op0 / op1;
				remain = op0 % op1;
				if (!res)
					backup_CR[9] |= COPRO_Z;
				else
					backup_CR[9] &= ~COPRO_Z;
				backup_CR[9] &= ~COPRO_C;
			}
		}
		for (int i = 0; i < 4; i++) {
			backup_CR[i] = (res >> (i * 8)) & 0xFF;
			backup_CR[i + 4] = (remain >> (i * 8)) & 0xFF;
		}
	}

	void Coprocessor::Tick() {
		if (!(emulator.chipset.cpu.reg_cr[9] & 0x01))
			return;

		if (tick_counter <= 0) {
			tick_counter = 0;

			//Write back results
			switch (backup_CR[8] & 0x07)
			{
			case 5:
				emulator.chipset.cpu.reg_cr[6] = backup_CR[6];
				emulator.chipset.cpu.reg_cr[7] = backup_CR[7];
			case 1:
				emulator.chipset.cpu.reg_cr[4] = backup_CR[4];
				emulator.chipset.cpu.reg_cr[5] = backup_CR[5];
			default:
				for(int i = 0; i < 4; i++)
					emulator.chipset.cpu.reg_cr[i] = backup_CR[i];
				break;
			}

			//Write back flags
			uint8_t flag_mask = 0;
			switch (backup_CR[8] & 0x07)
			{
			case 0:
				if (backup_CR[8] & 0x10)
					flag_mask = COPRO_Z | COPRO_S;
				else
					flag_mask = COPRO_Z;
				break;
			case 1:
			case 5:
				if (backup_CR[8] & 0x10)
					flag_mask = COPRO_C | COPRO_Z | COPRO_S | COPRO_OV;
				else
					flag_mask = COPRO_C | COPRO_Z;
				break;
			case 2:
				flag_mask = COPRO_C | COPRO_Z | COPRO_S | COPRO_OV;
				break;
			case 3:
				flag_mask = COPRO_C | COPRO_Z | COPRO_S | COPRO_OV | COPRO_Q;
				break;
			default:
				break;
			}
			emulator.chipset.cpu.reg_cr[9] &= ~flag_mask;
			emulator.chipset.cpu.reg_cr[9] |= backup_CR[9] & flag_mask;
			emulator.chipset.cpu.reg_cr[9] &= 0xF8;
		}

		tick_counter--;
	}

	void Coprocessor::Refresh() {
		if (start_operation) {
			for (int i = 0; i < 16; i++)
				backup_CR[i] = emulator.chipset.cpu.reg_cr[i];
			emulator.chipset.cpu.reg_cr[9] |= 1;
			switch (emulator.chipset.cpu.reg_cr[8] & 0x07)
			{
			case 0:
				Mul16();
				tick_counter = 4;
				break;
			case 1:
				Div16();
				tick_counter = 8;
				break;
			case 2:
				MAC(false);
				tick_counter = 4;
				break;
			case 3:
				MAC(true);
				tick_counter = 4;
				break;
			case 5:
				//MSB of coprocessor marks if 32bit/32bit division is enabled.
				if (copro_id & 0x80) {
					Div32();
					tick_counter = 16;
					break;
				}
			default:
				emulator.chipset.cpu.reg_cr[9] &= 0xF8;
				break;
			}
			start_operation = false;
		}
	}

	void Coprocessor::Reset() {
		for (int i = 0; i < 15; i++)
			emulator.chipset.cpu.reg_cr[i] = 0;

		emulator.chipset.cpu.reg_cr[15] = copro_id;

		tick_counter = 0;
		start_operation = false;
	}

	void Coprocessor::SetCR(int index, uint8_t data) {
		//Write to CR7 starts the operaton if CLEN bit is set to 1.
		//This should always occur at the last execution cycle of the coprocessor data transfer instruction.
		if (index == 7 && (emulator.chipset.cpu.reg_cr[8] & 0x80) && !(emulator.chipset.cpu.reg_cr[9] & 0x01))
			start_operation = true;

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