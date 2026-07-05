#pragma once

#include "imgui.h"

namespace gm_laboratory {

	struct ImGuiOverlay {
		static void AddInitCallback(void (*callback)());
		static void AddFrameCallback(void (*callback)());
		static ImGuiContext* GetContext();
		static void GetAllocatorFns(ImGuiMemAllocFunc* alloc, ImGuiMemFreeFunc* freeFn, void** userData);
	};

}
