﻿#pragma once

#include "../Emulator.hpp"
#include "imgui/imgui.h"
#include "hex.hpp"
#include <cstdint>

class WatchWindow {
private:
    uint8_t reg_rx[16][3];
    char reg_lr[5],reg_sp[5],reg_ea[5],reg_pc[5],reg_psw[3];
    int char_width;
    MemoryEditor mem_editor;
    bool isbreaked = false;
public:

    WatchWindow();

    void Show();

    void ShowRX();

    void PrepareRX();

    void UpdateRX();
};