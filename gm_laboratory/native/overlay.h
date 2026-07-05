#pragma once

#include "imgui.h"

struct ImPlotContext;
struct ImPlot3DContext;

namespace gm_laboratory {

	struct ImGuiOverlay {
		static void AddInitCallback(void (*callback)());
		static void AddFrameCallback(void (*callback)());
		static ImGuiContext* GetContext();
		static ImPlotContext* GetImPlotContext();
		static ImPlot3DContext* GetImPlot3DContext();
		static void GetAllocatorFns(ImGuiMemAllocFunc* alloc, ImGuiMemFreeFunc* freeFn, void** userData);
	};

}
