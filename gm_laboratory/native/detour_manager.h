#pragma once

#include <cstddef>

namespace gm_laboratory {

struct DetourSetupContext {
	bool NotSupported = false;

	void* AddDetour(const char* module, const char* pattern, void* detour, std::size_t offset = 0);
	void* AddDetourExport(const char* module, const char* exportName, void* detour);
};

struct IImplementsDetours {
	virtual ~IImplementsDetours() = default;
	virtual void SetupWin64(DetourSetupContext& ctx) { ctx.NotSupported = true; }
};

void RegisterDetour(IImplementsDetours* instance);

void* GetOrLoadModule(const char* name);

class DetourManager {
public:
	static void Bootstrap();
};

}

#define REGISTER_DETOUR(TypeName)                                                       \
	namespace {                                                                         \
	struct TypeName##_Registrar {                                                       \
		TypeName##_Registrar() { ::gm_laboratory::RegisterDetour(new TypeName()); }      \
	} g_##TypeName##_Registrar;                                                         \
	}
