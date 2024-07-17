#pragma once
#include "../Config.hpp"

#include <SDL.h>

namespace casioemu
{
	class Emulator;

	enum ClockType
	{
		CLOCK_UNDEFINED = 0,
		CLOCK_LSCLK = 1,
		CLOCK_HSCLK = 2,
		CLOCK_STOPPED = 3
	};

	class Peripheral
	{
	protected:
		Emulator &emulator;

		/**
		 * This should be true if the state of this peripheral changed
		 * so that it requires a call to Frame().
		 * It should not directly call Frame() because otherwise it may
		 * call it more than required (once per timer_interval)
		 */
		bool require_frame;

		//Set this value for peripherals controlled by BLKCON
		bool enabled = false;

		int clock_type = CLOCK_UNDEFINED;
		int block_bit = -1;

	public:
		Peripheral(Emulator &emulator);
		virtual void Initialise();
		virtual void Uninitialise();
		virtual void Tick();
		virtual void TickAfterInterrupts();
		virtual void Frame();
		virtual void UIEvent(SDL_Event &event);
		virtual void Reset();
		virtual bool GetRequireFrame();
		virtual void ResetLSCLK();
		virtual int GetClockType();
		virtual int GetBlockBit();
		virtual ~Peripheral();
	};
}

