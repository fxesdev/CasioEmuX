#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class Flash : public Peripheral
	{
		MMURegion region_FLASHA, region_FLASHD, region_FLASHCON, region_FLASHACP, region_FLASHSEG, region_FLASHSTA;
		uint16_t FLASHA, FLASHD;
		uint8_t FLASHACP_last, FLASHSEG;
		bool FLASHERR;

		bool* FlashStat[256];
		int SegmentSize[256] = { 0 };

		bool FlashLock;

		int TickCounter, OperationTime;

		int BlockSize = 0x4000;
		const int WriteTime = 167;
		const int EraseTime = 104857;

	public:
		using Peripheral::Peripheral;

		//1 for read, 4 for write, 5 for block erase, 6 for chip erase
		int FlashMode;

		uint16_t ReadWord(uint8_t seg, uint16_t offset);
		void Write(uint8_t seg, uint16_t offset, uint16_t data);
		void BlockErase(uint8_t seg, uint16_t addr);
		void ChipErase();

		void Initialise();
		void Tick();
		void Reset();
	};
}
