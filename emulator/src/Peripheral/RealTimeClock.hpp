#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class RealTimeClock : public Peripheral
	{
		MMURegion region_RTCSEC, region_RTCMIN, region_RTCHOUR, region_RTCWEEK, region_RTCDAY, region_RTCMON, region_RTCYEAR,
                  region_RTCCON, region_AL0MIN, region_AL0HOUR, region_AL0WEEK, region_AL1MIN, region_AL1HOUR, region_AL1DAY, region_AL1MON;
        
        uint8_t RTCSEC, RTCMIN, RTCHOUR, RTCWEEK, RTCDAY, RTCMON, RTCYEAR, RTCCON, AL0MIN, AL0HOUR, AL0WEEK, AL1MIN, AL1HOUR, AL1DAY, AL1MON;
        
        size_t RTCINT = 14;
        size_t AL0INT = 15;
        size_t AL1INT = 16;

        bool RTCSEC_carry;
        
        const uint8_t day_count[0x12] = {0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x30, 0x31};
        const uint8_t day_count_leap[0x12] = {0x31, 0x29, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x30, 0x31};

        template<uint8_t RealTimeClock::*value_ptr, uint8_t mask = 0xFF>
        static uint8_t RTCRead(MMURegion* region, size_t) {
            RealTimeClock* self = (RealTimeClock*)region->userdata;
            return self->*value_ptr & mask;
        }

        template<uint8_t RealTimeClock::*value_ptr, uint8_t mask = 0xFF, uint8_t minimum_val = 0>
        static void RTCWrite(MMURegion* region, size_t, uint8_t data) {
            RealTimeClock* self = (RealTimeClock*)region->userdata;
            if(self->RTCCON & 1)
                return;
            data &= mask;
            if(data < minimum_val)
                data = mask;
            self->*value_ptr = data;
        }

	public:
		using Peripheral::Peripheral;

        void CheckValue();
        void RTCTick();
        bool AL0Check();
        bool AL1Check();

		void Initialise();
		void Tick();
	};
}
