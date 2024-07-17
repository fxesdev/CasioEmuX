#include "CPU.hpp"

#include "../Emulator.hpp"
#include "Chipset.hpp"
#include "MMU.hpp"

namespace casioemu
{
	// * Load/Store Instructions
	void CPU::OP_LS_EA()
	{
		LoadStore(reg_ea, impl_hint >> 8, cpu_model == CM_NX_U16 ? true : false);
	}

	void CPU::OP_LS_R()
	{
		LoadStore(impl_operands[1].value, impl_hint >> 8, cpu_model == CM_NX_U16 ? true : false);
	}

	void CPU::OP_LS_I_R()
	{
		if (cpu_model == CM_NX_U16 && !(impl_hint & H_ST) && DSR_prefix_flag)
			cycle_counter++;

		LoadStore(impl_operands[1].value + impl_long_imm, impl_hint >> 8, false);
	}

	void CPU::OP_LS_BP()
	{
		if (cpu_model == CM_NX_U16 && !(impl_hint & H_ST) && DSR_prefix_flag)
			cycle_counter++;

		impl_operands[1].value |= (impl_operands[1].value & 0x20) ? 0xFFC0 : 0;
		impl_operands[1].value += reg_r[12] | (((uint16_t)reg_r[13]) << 8);
		LoadStore(impl_operands[1].value, impl_hint >> 8, false);
	}

	void CPU::OP_LS_FP()
	{
		if (cpu_model == CM_NX_U16 && !(impl_hint & H_ST) && DSR_prefix_flag)
			cycle_counter++;

		impl_operands[1].value |= (impl_operands[1].value & 0x20) ? 0xFFC0 : 0;
		impl_operands[1].value += reg_r[14] | (((uint16_t)reg_r[15]) << 8);
		LoadStore(impl_operands[1].value, impl_hint >> 8, false);
	}

	void CPU::OP_LS_I()
	{
		LoadStore(impl_long_imm, impl_hint >> 8, false);
	}

	void CPU::LoadStore(uint16_t offset, size_t length, bool access_by_word)
	{
		if (length % 2 == 0)
			offset &= ~1;
		size_t reg_base = impl_operands[0].value;
		if (impl_hint & H_ST)
		{
			for (size_t ix = length - 1; ix != (size_t)-1; --ix)
				emulator.chipset.mmu.WriteData((((size_t)reg_dsr) << 16) | (uint16_t)(offset + ix), reg_r[reg_base + ix]);
		}
		else
		{
			if (access_by_word && length > 1) {
				for (size_t ix = 0; ix < length; ix += 2)
				{
					uint16_t data = emulator.chipset.mmu.ReadWord((((size_t)reg_dsr) << 16) | (uint16_t)(offset + ix));

					impl_operands[0].value = data & 0xFF;
					ZSCheck();
					reg_r[reg_base + ix] = impl_operands[0].value;

					impl_operands[0].value = (data >> 8) & 0xFF;
					ZSCheck();
					reg_r[reg_base + ix + 1] = impl_operands[0].value;
				}
			}
			else {
				for (size_t ix = 0; ix != length; ++ix)
				{
					impl_operands[0].value = emulator.chipset.mmu.ReadData((((size_t)reg_dsr) << 16) | (uint16_t)(offset + ix));
					ZSCheck(); // * defined in CPUArithmetic.cpp
					reg_r[reg_base + ix] = impl_operands[0].value;
				}
			}
		}

		if (impl_hint & H_IA)
			BumpEA(length); // * defined in CPUControl.cpp
	}
}

