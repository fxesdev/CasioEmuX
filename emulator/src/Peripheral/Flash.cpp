#include "Flash.hpp"

#include "../Logger.hpp"
#include "../Emulator.hpp"
#include "../Chipset/CPU.hpp"
#include "../Chipset/MMU.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
	void Flash::Initialise() {
		clock_type = CLOCK_UNDEFINED;

		//initialize flash size
		switch (emulator.hardware_id)
		{
		case HW_CLASSWIZ:
			for (int i = 0; i < 4; i++)
				SegmentSize[i] = 0x10000;
			SegmentSize[0x1F] = 0x1000;
			break;
		case HW_CLASSWIZ_II:
			for (int i = 0; i < 6; i++)
				SegmentSize[i] = 0x10000;
			SegmentSize[7] = 0x2000;
			SegmentSize[0x1F] = 0x1000;
			break;
		case HW_FX_5800P:
			for (int i = 0; i < 16; i++)
				SegmentSize[i] = 0x10000;
			break;
		default:
			break;
		}

		for (int i = 0; i < 256; i++) {
			if (SegmentSize[i] > 0)
			{
				FlashStat[i] = new bool[SegmentSize[i] / 2];
				memset(FlashStat[i], 0, SegmentSize[i] / 2 * sizeof(bool));
			}
		}

		Reset();

		if (emulator.hardware_id == HW_FX_5800P)
			return;

		region_FLASHA.Setup(0xF0E0, 2, "Flash/FLASHA", &FLASHA, MMURegion::DefaultRead<uint16_t, 0xFFFE>, MMURegion::DefaultWrite<uint16_t, 0xFFFE>, emulator);
		region_FLASHD.Setup(0xF0E2, 2, "Flash/FLASHD", this, [](MMURegion* region, size_t offset) {
			offset -= region->base;
			Flash* flash = (Flash*)region->userdata;
			return (uint8_t)((flash->FLASHD >> offset * 8) & 0xFF);
		}, [](MMURegion* region, size_t offset, uint8_t data) {
			offset -= region->base;
			Flash* flash = (Flash*)region->userdata;
			flash->FLASHD &= ~(0xFF << offset * 8);
			flash->FLASHD |= data << offset * 8;
			if (offset == 1 && !flash->FlashLock)
				flash->Write(flash->FLASHSEG, flash->FLASHA, flash->FLASHD);
		}, emulator);
		region_FLASHCON.Setup(0xF0E4, 1, "Flash/FLASHCON", this, MMURegion::IgnoreRead<0x00>, [](MMURegion* region, size_t, uint8_t data) {
			Flash* flash = (Flash*)region->userdata;
			if (data & 0x01 && !flash->FlashLock)
				flash->BlockErase(flash->FLASHSEG, flash->FLASHA);
		}, emulator);
		region_FLASHACP.Setup(0xF0E5, 1, "Flash/FLASHACP", this, MMURegion::IgnoreRead<0x00>, [](MMURegion* region, size_t, uint8_t data) {
			Flash* flash = (Flash*)region->userdata;
			if (data == 0xF5 && flash->FLASHACP_last == 0xFA)
				flash->FlashLock = false;
			flash->FLASHACP_last = data;
		}, emulator);
		region_FLASHSEG.Setup(0xF0E6, 1, "Flash/FLASHSEG", &FLASHSEG, MMURegion::DefaultRead<uint8_t, 0x1F>, MMURegion::DefaultWrite<uint8_t, 0x1F>, emulator);
		region_FLASHSTA.Setup(0xF0EF, 1, "Flash/FLASHSTA", &FLASHERR, MMURegion::DefaultRead<uint8_t>, [](MMURegion* region, size_t, uint8_t) {
			*((bool*)region->userdata) = false;
		}, emulator);
	}

	uint16_t Flash::ReadWord(uint8_t seg, uint16_t offset) {
		if (offset >= SegmentSize[seg])
			return 0;

		offset &= 0xFFFE;
		int real_offset = (seg << 16) | offset;

		switch (emulator.hardware_id)
		{
		case HW_CLASSWIZ:
			if (!FlashStat[seg][offset / 2])
			{
				return (((uint16_t)emulator.chipset.rom_data[real_offset + 1]) << 8) | emulator.chipset.rom_data[real_offset];
			}
			else
			{
				if (!emulator.realistic_simulation)
					return 0;
				//handle erased data here; they are kind of randomized in real hardware.
				return 0;
			}
		case HW_CLASSWIZ_II:
			//7:0000-7:1FFF might be a mirror of 5:E000 to 5:FFFF;so it might be only necessary to provide a 384kb rom file.
			if (seg == 7)
				real_offset -= 0x12000;
			if (!FlashStat[seg][offset / 2])
			{
				return (((uint16_t)emulator.chipset.rom_data[real_offset + 1]) << 8) | emulator.chipset.rom_data[real_offset];
			}
			else
			{
				if (!emulator.realistic_simulation)
					return 0;
				return 0;
			}
		case HW_FX_5800P:
			real_offset += 0x80000;
			return (((uint16_t)emulator.chipset.rom_data[real_offset + 1]) << 8) | emulator.chipset.rom_data[real_offset];
		default:
			PANIC("Trying to access flash in model with no flash!");
		}
		return 0;
	}

	void Flash::Write(uint8_t seg, uint16_t offset, uint16_t data) {
		FlashMode = 4;
		emulator.chipset.cpu.cpu_run_stat = false;
		offset &= 0xFFFE;
		if (FlashStat[seg][offset / 2]) {
			OperationTime = WriteTime;
			FlashStat[seg][offset / 2] = false;
			int real_offset = (seg << 16) | offset;
			emulator.chipset.rom_data[real_offset] = (uint8_t)(data & 0xFF);
			emulator.chipset.rom_data[real_offset + 1] = (uint8_t)((data >> 8) & 0xFF);
		}
		FlashLock = true;
	}

	void Flash::BlockErase(uint8_t seg, uint16_t addr) {
		FlashMode = 5;
		emulator.chipset.cpu.cpu_run_stat = false;
		if (addr < SegmentSize[seg]) {
			OperationTime = EraseTime;
			addr &= ~(BlockSize - 1);
			for (int i = addr / 2; i < std::min((addr + BlockSize) / 2, SegmentSize[seg] / 2); i++)
				FlashStat[seg][i] = true;
		}
		FlashLock = true;
	}

	void Flash::ChipErase() {
		FlashMode = 6;
		OperationTime = EraseTime;
		emulator.chipset.cpu.cpu_run_stat = false;
		for (int i = 0; i < 256; i++) {
			if (SegmentSize[i] > 0)
			{
				for (int j = 0; j < SegmentSize[i] / 2; j++)
					FlashStat[i][j] = true;
			}
		}
		FlashLock = true;
	}

	void Flash::Tick() {
		if (FlashMode != 1) {
			if (++TickCounter >= OperationTime) {
				TickCounter = OperationTime = 0;
				FlashMode = 1;
				emulator.chipset.cpu.cpu_run_stat = true;
			}
		}
	}

	void Flash::Reset() {
		FLASHA = FLASHD = 0;
		FLASHACP_last = FLASHSEG = 0;
		FLASHERR = false;
		FlashLock = true;
		FlashMode = 1;
		TickCounter = OperationTime = 0;
	}
}