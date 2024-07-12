#pragma once
#include "imgui/imgui.h"
#include "hex.hpp"
class AppMemoryView {
	MemoryEditor mem_edit;
	MemoryEditor::OptionalMarkedSpans spans{ std::vector<MemoryEditor::MarkedSpan>{
	} };

public:
	AppMemoryView();
	void Draw();
};
