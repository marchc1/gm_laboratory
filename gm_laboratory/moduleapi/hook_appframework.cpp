#include "Detours.h"

#include <vector>

#include "tier1/interface.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "tier3/tier3.h"

#include "moduleapi/hooks.h"
#include "native/detour_manager.h"
#include "native/interface_registry.h"
#include "native/log.h"

#include "defs/dll_names.h"
#include "defs/function_signatures.h"
#include "defs/sourcesdk_help.h"

namespace gm_laboratory {

using BootAppSystemGroup__Create = bool (*)(void*);
using CModAppSystemGroup__Main = int (*)(void*);

static BootAppSystemGroup__Create BootAppSystemGroup__Create_Original;
static CModAppSystemGroup__Main CModAppSystemGroup__Main_Original;


static void WalkInterfacesRichly() {
	WalkInterfaces(
		[](const char* moduleName, void* ptr, void* createInterfaceFn) {
			Log("interfaces", "[%p]  %s (CreateInterface @ %p)\n", ptr, moduleName, createInterfaceFn);
		},
		[](const char* interfaceName, void* ptr) {
			Log("interfaces", "    - [%p]      %s\n", ptr, interfaceName);
		});
}

static bool BootAppSystemGroup__Create_Detour(void* self) {
	Hooks::PreBootAppSystemCreate.Invoke();
	bool r = BootAppSystemGroup__Create_Original(self);
	Hooks::PostBootAppSystemCreate.Invoke();

	return r;
}

static int CModAppSystemGroup__Main_Detour(void* self) {
	SOURCE_SDK_CONNECT()

	Hooks::PreModAppSystemMain.Invoke();
	ConnectGameLibraries();
	WalkInterfacesRichly();

	int result = CModAppSystemGroup__Main_Original(self);

	return result;
}

static std::string DemangleRTTIName(const char* mangled) {
	if (!mangled || !*mangled) {
		return "<unknown>";
	}

	std::string name = mangled;

	size_t start = 0;
	if (name.size() > 4 && name[0] == '.' && name[1] == '?' && name[2] == 'A') 
		start = 4; 

	size_t end = name.size();
	if (end >= 2 && name[end - 1] == '@' && name[end - 2] == '@') 
		end -= 2; 

	if (start >= end) 
		return mangled;

	std::vector<std::string> scopes;
	for (size_t pos = start; pos < end;) {
		size_t at = name.find('@', pos);
		if (at == std::string::npos || at > end) {
			at = end;
		}
		scopes.push_back(name.substr(pos, at - pos));
		pos = at + 1;
	}

	std::string result;
	for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
		if (it->empty()) {
			continue;
		}
		if (!result.empty()) {
			result += "::";
		}
		result += *it;
	}

	return result.empty() ? mangled : result;
}

static void DumpRTTIObject(void* moduleBase, Detours::RTTI::Object* object, int depth) {
	if (!object) {
		return;
	}

	const auto pTypeDescriptor = object->GetTypeDescriptor();
	std::string name = pTypeDescriptor ? DemangleRTTIName(pTypeDescriptor->m_szName) : "<no type descriptor>";

	void** vtable = object->GetVTable();
	if (vtable) {
		Log("rtti", "%*s%s  (vtable @ %p, +0x%zX)\n", depth * 2, "", name.c_str(), reinterpret_cast<void*>(vtable),
			static_cast<size_t>(reinterpret_cast<uintptr_t>(vtable) - reinterpret_cast<uintptr_t>(moduleBase)));
	} else {
		Log("rtti", "%*s%s\n", depth * 2, "", name.c_str());
	}

	for (auto& base : object->GetBaseObjects()) {
		DumpRTTIObject(moduleBase, base.get(), depth + 1);
	}
}

static void Hooks_PostLoadLibrary(const char* filename, void* returned) {
	Hooks::ExecRewriteInterfaces(GetInterfaceRegistry(returned));

	/*
	auto rtti = Detours::RTTI::DumpRTTI(filename);
	Log("rtti", "==== RTTI dump for %s (%zu objects) ====\n", filename, rtti.size());
	for (auto& rttiItem : rtti) {
		DumpRTTIObject(returned, rttiItem.get(), 0);
	}
	*/
}

struct PlugIntoAppFramework : IImplementsDetours {
	void SetupWin64(DetourSetupContext& ctx) override {
		BootAppSystemGroup__Create_Original = reinterpret_cast<BootAppSystemGroup__Create>(ctx.AddDetour(LAUNCHER_DLL, BootAppSystemGroup__Create_SIG, reinterpret_cast<void*>(&BootAppSystemGroup__Create_Detour)));
		CModAppSystemGroup__Main_Original = reinterpret_cast<CModAppSystemGroup__Main>(ctx.AddDetour(ENGINE_DLL, CModAppSystemGroup__Main_SIG, reinterpret_cast<void*>(&CModAppSystemGroup__Main_Detour)));

		Hooks::PostLoadLibrary += Hooks_PostLoadLibrary;
	}
};

REGISTER_DETOUR(PlugIntoAppFramework)

}
