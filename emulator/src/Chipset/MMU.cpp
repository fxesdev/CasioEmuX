#include "MMU.hpp"

#include <vector>
#include <cstring>
#include <algorithm>
#include "../Emulator.hpp"
#include "Chipset.hpp"
#include "../Logger.hpp"
#include "../Gui/ui.hpp"
#include "CPU.hpp"

namespace casioemu
{
	MMU::MMU(Emulator &_emulator) : emulator(_emulator)
	{
		segment_dispatch = new MemoryByte *[0x100];
		for (size_t ix = 0; ix != 0x100; ++ix)
			segment_dispatch[ix] = nullptr;
	}

	MMU::~MMU()
	{
		for (size_t ix = 0; ix != 0x100; ++ix)
			if (segment_dispatch[ix])
				delete[] segment_dispatch[ix];

		delete[] segment_dispatch;
	}

	void MMU::GenerateSegmentDispatch(size_t segment_index)
	{
		segment_dispatch[segment_index] = new MemoryByte[0x10000];
		for (size_t ix = 0; ix != 0x10000; ++ix)
		{
			segment_dispatch[segment_index][ix].region = nullptr;
			segment_dispatch[segment_index][ix].on_read = LUA_REFNIL;
			segment_dispatch[segment_index][ix].on_write = LUA_REFNIL;
		}
	}

	void MMU::SetupInternals()
	{
		me_mmu = this;
		real_hardware = emulator.GetModelInfo("real_hardware");

		emulator.chipset.SegmentAccess = false;

		*(MMU **)lua_newuserdata(emulator.lua_state, sizeof(MMU *)) = this;
		lua_newtable(emulator.lua_state);
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
			lua_pushinteger(lua_state, mmu->ReadCode(lua_tointeger(lua_state, 2)));
			return 1;
		});
		lua_setfield(emulator.lua_state, -2, "__index");
		lua_pushcfunction(emulator.lua_state, [](lua_State *) {
			return 0;
		});
		lua_setfield(emulator.lua_state, -2, "__newindex");
		lua_setmetatable(emulator.lua_state, -2);
		lua_setglobal(emulator.lua_state, "code");

		*(MMU **)lua_newuserdata(emulator.lua_state, sizeof(MMU *)) = this;
		lua_newtable(emulator.lua_state);
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
			int isnum;
			size_t offset = lua_tointegerx(lua_state, 2, &isnum);
			if (isnum)
			{
				lua_pushinteger(lua_state, mmu->ReadData(offset));
				return 1;
			}
			const char *key = lua_tostring(lua_state, 2);
			if (key == nullptr)  // the key is not a string
				return 0;
			if (std::strcmp(key, "rwatch") == 0)
			{
				// execute Lua function whenever address is read from
				lua_pushcfunction(lua_state, [](lua_State *lua_state) {
					if (lua_gettop(lua_state) != 3)
						return luaL_error(lua_state, "rwatch function called with incorrect number of arguments");

					MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
					size_t offset = lua_tointeger(lua_state, 2);
					int on_read = luaL_ref(lua_state, LUA_REGISTRYINDEX);

					size_t segment_index = offset >> 16;
					size_t segment_offset = offset & 0xFFFF;

					MemoryByte *segment = mmu->segment_dispatch[segment_index];
					if (!segment)
					{
						logger::Info("attempt to set rwatch from offset %04zX of unmapped segment %02zX\n",
								segment_offset, segment_index);
						return 0;
					}

					MemoryByte &byte = segment[segment_offset];
					luaL_unref(lua_state, LUA_REGISTRYINDEX, byte.on_read);
					byte.on_read = on_read;
					return 0;
				});
				return 1;
			}
			else if (std::strcmp(key, "watch") == 0)
			{
				// execute Lua function whenever address is written to
				lua_pushcfunction(lua_state, [](lua_State *lua_state) {
					if (lua_gettop(lua_state) != 3)
						return luaL_error(lua_state, "watch function called with incorrect number of arguments");

					MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
					size_t offset = lua_tointeger(lua_state, 2);
					int on_write = luaL_ref(lua_state, LUA_REGISTRYINDEX);

					size_t segment_index = offset >> 16;
					size_t segment_offset = offset & 0xFFFF;

					MemoryByte *segment = mmu->segment_dispatch[segment_index];
					if (!segment)
					{
						logger::Info("attempt to set watch from offset %04zX of unmapped segment %02zX\n",
								segment_offset, segment_index);
						return 0;
					}

					MemoryByte &byte = segment[segment_offset];
					luaL_unref(lua_state, LUA_REGISTRYINDEX, byte.on_write);
					byte.on_write = on_write;
					return 0;
				});
				return 1;
			}
			else if (std::strcmp(key, "addr_region") == 0)
			{
				// execute Lua function whenever address is written to
				lua_pushcfunction(lua_state, [](lua_State *lua_state) {
					if (lua_gettop(lua_state) != 2)
						return luaL_error(lua_state, "addr_region function called with incorrect number of arguments");

					MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
					size_t offset = lua_tointeger(lua_state, 2);

					MMURegion *region = mmu->GetAddressRegion(offset);
					if (region) logger::Info("Data memory address %02X:%04XH is mapped to region %s\n", offset >> 16, offset & 0xFFFF, region->description.c_str());
					else logger::Info("Data memory address %02X:%04XH is not mapped to any region\n", offset >> 16, offset & 0xFFFF);
					return 0;
				});
				return 1;
			}
			else
			{
				return 0;
			}
		});
		lua_setfield(emulator.lua_state, -2, "__index");
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
			mmu->WriteData(lua_tointeger(lua_state, 2), lua_tointeger(lua_state, 3));
			return 0;
		});
		lua_setfield(emulator.lua_state, -2, "__newindex");
		lua_setmetatable(emulator.lua_state, -2);
		lua_setglobal(emulator.lua_state, "data");
	}

	uint16_t MMU::ReadCode(size_t offset)
	{
		if (offset >= (1 << 20))
			PANIC("offset doesn't fit 20 bits\n");
		if (offset & 1)
			PANIC("offset has LSB set\n");

		uint8_t segment_index = offset >> 16;
		uint16_t segment_offset = offset & 0xFFFF;

		switch (emulator.hardware_id) {
		case HW_ES_PLUS:
			offset &= 0x1FFFE;
			if (offset < emulator.chipset.rom_data.size())
				return (((uint16_t)emulator.chipset.rom_data[offset + 1]) << 8) | emulator.chipset.rom_data[offset];
			else
				return 0;
		case HW_CLASSWIZ:
			if (emulator.chipset.SegmentAccess && segment_index == 5)
				segment_index = 0;
			if (segment_index < 4) {
				if (emulator.chipset.HWRemap)
					return (segment_index == 0 && segment_offset < 0x200) ? emulator.chipset.flash->ReadWord(0, segment_offset + 0xFE00) : emulator.chipset.flash->ReadWord(segment_index, segment_offset);
				else
					return (segment_index == 0 && segment_offset >= 0xFE00) ? 0xFFFF : emulator.chipset.flash->ReadWord(segment_index, segment_offset);
			}
			return 0;
		case HW_CLASSWIZ_II:
			if (segment_index == 8)
				return emulator.chipset.flash->ReadWord(0, segment_offset);
			segment_index &= 7;
			if (segment_index == 6 || (segment_index == 7 && segment_offset >= 0x2000))
				return 0xFFFF;
			if (emulator.chipset.HWRemap)
				return (segment_index == 0 && segment_offset < 0x200) ? emulator.chipset.flash->ReadWord(0, segment_offset + 0xFE00) : emulator.chipset.flash->ReadWord(segment_index, segment_offset);
			else
				return (segment_index == 0 && segment_offset >= 0xFE00) ? 0xFFFF : emulator.chipset.flash->ReadWord(segment_index, segment_offset);
		case HW_FX_5800P:
			if(segment_index < 2)
				return (((uint16_t)emulator.chipset.rom_data[offset + 1]) << 8) | emulator.chipset.rom_data[offset];
			if (segment_index >= 8)
				return emulator.chipset.flash->ReadWord(segment_index & 7, segment_offset);
			return 0xFFFF;
		default:
			PANIC("Unknown hardware id!");
		}
	}

	uint8_t MMU::ReadData(size_t offset, bool softwareRead)
	{
		if (offset >= (1 << 24))
			PANIC("offset doesn't fit 24 bits\n");

		//now we find out DSR is also masked with 0x1F in classwiz ii
		//yet not clear about exact behavior of the cpu on accessing segment 10 to 1F
		/*
		things about accessing unmapped segment is actually far more complex on real hardware;
		the result seems to be also affected by the next instruction,
		and not every address of a certain segment gives the same result.
		the code here only provides a simple simulation which could deal with 2 qr code errors on fx991cncw:
		1.x=an in solver,exe,page down,exe twice,then catalog;
		2.1234567890123xan in solver,exe,page down,exe,catalog,up
		*/
		if(emulator.hardware_id == HW_CLASSWIZ_II && real_hardware) {
			offset = getRealOffset(offset);
			switch (offset)
			{
			case 0x1000001:
				emulator.chipset.cpu.CorruptByDSR();
				return 0;
			case 0x1000002:
				return 0;
			case 0x1000003:
				return 0xFF;
			default:
				break;
			}
		}
		
		size_t segment_index = offset >> 16;
		size_t segment_offset = offset & 0xFFFF;

		MemoryByte *segment = segment_dispatch[segment_index];
		if (!segment)
		{
			//logger::Info("read from offset %04zX of unmapped segment %02zX\n", segment_offset, segment_index);
			emulator.HandleMemoryError();
			return 0;
		}

		MemoryByte &byte = segment[segment_offset];
		MMURegion *region = byte.region;
		if (byte.on_read != LUA_REFNIL && softwareRead)
		{
			lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, byte.on_read);
			if (lua_pcall(emulator.lua_state, 0, 0, 0) != LUA_OK)
			{
				//logger::Info("calling commands on rwatch at %06zX failed: %s\n",
						//offset, lua_tostring(emulator.lua_state, -1));
				lua_pop(emulator.lua_state, 1);
			}
		}
		if (!region)
		{
			//logger::Info("read from unmapped offset %04zX of segment %02zX\n", segment_offset, segment_index);
			emulator.HandleMemoryError();
			return 0;
		}

		return region->read(region, offset);
	}

	void MMU::WriteData(size_t offset, uint8_t data, bool softwareWrite)
	{
		if (offset >= (1 << 24))
			PANIC("offset doesn't fit 24 bits\n");

		size_t segment_index = offset >> 16;
		size_t segment_offset = offset & 0xFFFF;

		MemoryByte *segment = segment_dispatch[segment_index];
		if (!segment)
		{
			//logger::Info("write to offset %04zX of unmapped segment %02zX (%02zX)\n", segment_offset, segment_index, data);
			emulator.HandleMemoryError();
			return;
		}

		MemoryByte &byte = segment[segment_offset];
		MMURegion *region = byte.region;
		if (byte.on_write != LUA_REFNIL && softwareWrite)
		{
			lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, byte.on_write);
			if (lua_pcall(emulator.lua_state, 0, 0, 0) != LUA_OK)
			{
				logger::Info("calling commands on watch at %06zX failed: %s\n",
						offset, lua_tostring(emulator.lua_state, -1));
				lua_pop(emulator.lua_state, 1);
			}
		}
		if (!region)
		{
			//logger::Info("write to unmapped offset %04zX of segment %02zX (%02zX)\n", segment_offset, segment_index, data);
			emulator.HandleMemoryError();
			return;
		}

		region->write(region, offset, data);
	}

	size_t MMU::getRealOffset(size_t offset) {
		size_t segment_index = offset >> 16;
		if(segment_index < 0x10)
			return offset;
		if(segment_index == 0x10 || segment_index == 0x18)
			return 0x1000001;
		if((segment_index & 0x07) > 5) {
			if(offset & 0x02 || !(offset & 0xFF)) {
				return 0x1000002;
			} else {
				return 0x1000003;
			}
		}
		return offset & 0x0FFFFF; 
	}

	void MMU::RegisterRegion(MMURegion *region)
	{
		for (size_t ix = region->base; ix != region->base + region->size; ++ix)
		{
			if (segment_dispatch[ix >> 16][ix & 0xFFFF].region)
				PANIC("MMU region overlap at %06zX\n", ix);
			segment_dispatch[ix >> 16][ix & 0xFFFF].region = region;
		}
		regions.push_back(region);
	}

	void MMU::UnregisterRegion(MMURegion *region)
	{
		for (size_t ix = region->base; ix != region->base + region->size; ++ix)
		{
			if (!segment_dispatch[ix >> 16][ix & 0xFFFF].region)
				PANIC("MMU region double-hole at %06zX\n", ix);
			segment_dispatch[ix >> 16][ix & 0xFFFF].region = nullptr;
		}
		regions.erase(std::find(regions.begin(), regions.end(), region));
	}


	MMURegion *MMU::GetAddressRegion(size_t offset)
	{
		return segment_dispatch[offset >> 16][offset & 0xFFFF].region;
	}

	std::vector<MMURegion*> MMU::GetRegions() {
		return regions;
	}
}

