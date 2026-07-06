#include "native/detour_manager.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "Detours.h"

#include "native/log.h"
#include "native/sigscan.h"

#include "tier0/dbg.h"

namespace gm_laboratory {
	static std::vector<IImplementsDetours*>& Implementors() {
		static std::vector<IImplementsDetours*> implementors;
		return implementors;
	}

	static std::vector<Detours::Hook::InlineWrapperHook*>& InstalledHooks() {
		static std::vector<Detours::Hook::InlineWrapperHook*> hooks;
		return hooks;
	}

	static std::unordered_map<std::string, void*>& LoadedModules() {
		static std::unordered_map<std::string, void*> loadedModules;
		return loadedModules;
	}

	void RegisterDetour(IImplementsDetours* instance) {
		Implementors().push_back(instance);
	}

	void* GetOrLoadModule(const char* name) {
		std::string key = name;
		auto& loadedModules = LoadedModules();
		auto it = loadedModules.find(key);
		if (it != loadedModules.end())
			return it->second;

		HMODULE address = GetModuleHandleA(name);
		if (!address)
			address = LoadLibraryExA(name, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

		loadedModules[key] = address;
		return address;
	}

	static void* InstallHook(void* target, void* detour) {
		auto* hook = new Detours::Hook::InlineWrapperHook(target);
		InstalledHooks().push_back(hook);
		if (!hook->Hook(detour, false))
			return nullptr;
		return hook->GetTrampoline();
	}

	void* DetourSetupContext::AddDetour(const char* module, const char* pattern, void* detour, std::size_t offset) {
		void* moduleAddress = GetOrLoadModule(module);
		AssertMsg(moduleAddress, "module %s could not be found", module);
		if (!moduleAddress) {
			Log("detours", "Cannot find module %s\n", module);
			return nullptr;
		}

		void* target = FindSignature(moduleAddress, pattern, offset);
		AssertMsg(target, "module %s could not sigscan pattern %s", module, pattern);
		if (!target) {
			Log("detours", "Cannot find signature in %s\n", module);
			return nullptr;
		}

		void* ptr = InstallHook(target, detour);
		AssertMsg(ptr, "internal detouring error despite finding original function", module, pattern);
		return ptr;
	}

	void* DetourSetupContext::AddDetourExport(const char* module, const char* exportName, void* detour) {
		void* moduleAddress = GetOrLoadModule(module);
		AssertMsg(moduleAddress, "module %s could not be found", module);

		if (!moduleAddress)
			return nullptr;

		void* target = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(moduleAddress), exportName));
		AssertMsg(target, "module %s could not find export named %s", module, exportName);
		if (!target)
			return nullptr;

		return InstallHook(target, detour);
	}

	void* DetourSetupContext::AddDetourRaw(void* target, void* detour) {
		if (!target)
			return nullptr;
		return InstallHook(target, detour);
	}

	void DetourManager::Bootstrap() {
		Log("detours", "Bootstrapping detours...\n");

		for (IImplementsDetours* instance : Implementors()) {
			DetourSetupContext ctx;
			instance->SetupWin64(ctx);
			if (ctx.NotSupported)
				Log("detours", "A detour-class did not implement SetupWin64, ignoring.\n");
		}

		Log("detours", "Detours ready.\n");
	}
}
