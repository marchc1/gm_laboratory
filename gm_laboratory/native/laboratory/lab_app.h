#pragma once

#include "imgui.h"

namespace gm_laboratory {

	struct LabAppDesc {
		const char* id;
		const char* title;
		const char* shortLabel;
		ImU32 accent;
		ImTextureID icon;
		void (*draw)(bool* p_open);
		bool startOpen;
	};

}
