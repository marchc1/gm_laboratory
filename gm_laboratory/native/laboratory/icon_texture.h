#pragma once

#include "imgui.h"

namespace gm_laboratory {
	struct IconTexture {
		static void SetDevice(void* d3d9Device);
		static ImTextureID Load(const char* path);
		static void Clear();
	};
}
