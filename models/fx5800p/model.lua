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
	generate(50, 554, 55, 41, 5, 4, 63, 54, {
		0x02, '7', 0x12, '8', 0x22, '9', 0x32, 'Backspace', 0x70, 'Space',
		0x01, '4', 0x11, '5', 0x21, '6', 0x31, '*' , 0x67, '/',
		0x42, '1', 0x52, '2', 0x62, '3', 0x66, '+', 0x65, '-',
		0x41, '0', 0x51, '.', 0x61, 'e', 0x64, '' , 0x63, 'Return',
	})
	generate(46, 366, 48, 31, 6, 4, 54, 46, {
		0x06, 'F3', 0x16, 'F4', 0x26, 'Escape', 0x36, 'f', 0x46, 'c', 0x56, 's',
		0x05, '', 0x15, '', 0x25, '', 0x35, '', 0x45, '', 0x55, '',
		0x04, '', 0x14, '', 0x24, '', 0x34, '', 0x44, '', 0x54, '',
		0x03, '', 0x13, '', 0x23, '', 0x33, '', 0x43, '', 0x53, '',
	})
	-- generate( 40, 359, 48, 31, 2, 1,  54,  0, {0x06, 'F5', 0x16, 'F6',})
	-- generate(268, 359, 48, 31, 2, 1,  54,  0, {0x46, 'F7', 0x56, 'F8',})
	generate( 41,  29, 86, 27, 1, 1, 273,  0, {0xFF, 'r'})
	generate( 55,  283, 37, 37, 1, 1, 273,  0, {0x07, 'F1'})
	generate(114, 306, 37, 37, 1, 1, 162,  0, {0x17, 'F2'})
	generate(180, 278, 45, 60, 2, 1,  125,  0, {0x27, 'Left', 0x57, 'Right',})
	generate(232, 271, 71, 22, 1, 2,   0, 62, {0x37, 'Up', 0x47, 'Down',})

	emu:model({
		model_name = "fx-5800p",
		interface_image_path = "interface.png",
		rom_path = "rom.bin",
		flash_path = "flash.bin",
		hardware_id = 6,
		real_hardware = 1,
		pd_value = 1,
		csr_mask = 0x000f,
		rsd_interface = {0, 0, 400, 787, 0, 0},
		rsd_pixel = {93, 121,  3,  4,  55, 100},
		rsd_s     = {400,   0, 10, 14,  61-5, 88},
		rsd_a     = {400,  14, 11, 14,  70-5, 88},
		rsd_m     = {400,  28, 10, 14,  81-5, 88},
		rsd_sto   = {400,  42, 20, 14,  91-5, 88},
		rsd_rcl   = {400,  56, 19, 14, 110-5, 88},
		rsd_stat  = {400,  70, 24, 14, 130-5, 88},
		rsd_cmplx = {400,  84, 32, 14, 154-5, 88},
		rsd_mat   = {400,  98, 20, 14, 186-5, 88},
		rsd_vct   = {400, 112, 20, 14, 205-5, 88},
		rsd_d     = {400, 126, 12, 14, 225-5, 88},
		rsd_r     = {400, 140, 10, 14, 236-5, 88},
		rsd_g     = {400, 154, 11, 14, 246-5, 88},
		rsd_fix   = {400, 168, 17, 14, 257-5, 88},
		rsd_sci   = {400, 182, 16, 14, 273-5, 88},
		rsd_math  = {400, 196, 24, 14, 289-5, 88},
		rsd_down  = {400, 210, 10, 14, 313-5, 88},
		rsd_up    = {400, 224, 10, 14, 319-5, 88},
		rsd_disp  = {400, 238, 20, 14, 329-5, 88},
		ink_colour = {49, 49, 49},
		button_map = buttons
	})
end