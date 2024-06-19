do
	local buttons = {}
	local function generate(px, py, w, h, nx, ny, sx, sy, code)
		local cp = 1
		for iy = 0, ny - 1 do
			for ix = 0, nx - 1 do
				table.insert(buttons, {px + ix*sx, py + iy*sy, w, h, code[cp], code[cp+1]})
				cp = cp + 2
			end
		end
	end
	-- Refer to https://wiki.libsdl.org/SDL_Keycode for key names.
	emu:model({
		model_name = "Graph Light",
		interface_image_path = "interface.png",
		rom_path = "rom.bin",
		hardware_id = 5,
		real_hardware = 0,
		csr_mask = 0x000F,
		pd_value = 0x00,
		rsd_interface = {0, 0, 500, 1010, 0, 0},
		rsd_pixel = { 58, 190,  2,  2,  58, 190},
		rsd_s     = { 63, 1031,  25,  25,  63, 163},
		rsd_math  = { 98, 1036, 38,  18,  98, 168},
		rsd_d     = {98, 1010,  38,  22, 98, 142},
		rsd_r     = {139, 1010,  38,  22, 139, 142},
		rsd_g     = {181, 1010,  38,  22, 181, 142},
		rsd_fix   = {142, 1036, 34,  18, 142, 168},
		rsd_sci   = {184, 1036, 34,  18, 184, 168},
		rsd_fx    = {301, 1012, 34,  18, 301, 144},
		rsd_e     = {227, 1036, 14,  18, 227, 168},
		rsd_cmplx = {229, 1010, 10,  22, 229, 142},
		rsd_angle = {247, 1010, 20,  22, 247, 142},
		rsd_wdown = {247, 1036, 18,  18, 247, 168},
		rsd_verify = {272, 1035, 20, 20, 272, 167},
		rsd_gx    = {300, 1036, 34,  18, 300, 168},
		rsd_left  = {341, 1023, 14,  16, 341, 155},
		rsd_down  = {356, 1041, 16,  14, 356, 173},
		rsd_up    = {355, 1008, 16,  14, 355, 140},
		rsd_right = {373, 1023, 14,  16, 373, 155},
		rsd_pause = {393, 1024, 22,  18, 393, 156},
		rsd_sun   = {420, 1009, 22,  22, 420, 141},
		ink_colour = {0, 0, 0},
		button_map = buttons
	})
	generate(64, 688, 58, 58, 5, 4, 79, 66, {
		0x02, 'Keypad 7', 0x12, 'Keypad 8', 0x22, 'Keypad 9', 0x32, 'Backspace', 0x42, 'Space',
		0x01, 'Keypad 4', 0x11, 'Keypad 5', 0x21, 'Keypad 6', 0x31, 'Keypad *' , 0x41, 'Keypad /',
		0x00, 'Keypad 1', 0x10, 'Keypad 2', 0x20, 'Keypad 3', 0x30, 'Keypad +', 0x40, 'Keypad -',
		0x64, 'Keypad 0', 0x63, 'Keypad .', 0x62, '/', 0x61, 'Right Shift' , 0x60, 'Return',
	})
	generate(68, 576, 46, 46, 6, 2, 64, 56, {
		0x04, 'A', 0x14, 'S', 0x24, 'D', 0x34, 'J', 0x44, 'K', 0x54, 'L',
		0x03, 'Z', 0x13, 'X', 0x23, 'C', 0x33, 'B', 0x43, 'N', 0x53, 'M',
	})
	generate(68, 452, 46, 46, 6, 1, 64, 0, {
		0x06, 'Left Shift', 0x16, 'Escape', 0x26, 'Left', 0x36, '\\', 0x46, 'Right', 0x56, 'PageDown',
	})
	generate(68, 520, 46, 46, 3, 1,  64,  0, {0x05, 'Q', 0x15, 'W', 0x25, 'E',})
	generate(324, 520, 46, 46, 2, 1,  64,  0, {0x45, 'O', 0x55, 'P',})
	generate(260, 508, 46, 46, 1, 1,  0,  0, {0x35, 'Down',})
	generate(132, 396, 46, 46, 3, 1,  128,  0, {0x17, 'Home', 0x37, 'Up', 0x57, 'PageUp',})
	generate(71, 396, 38, 38, 1, 1, 0, 0, {0xFF, 'F1'})
end
