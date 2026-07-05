#pragma once
#include "globalusings.h"
#include <cstddef>
#include "client_class.h"
#include "server_class.h"

namespace gm_laboratory {
	constexpr unsigned int MODULE_ABI_VERSION = 2026070402;
	struct InterfaceRegistryEditor;

	struct ModuleAPI {
		unsigned int abiVersion;

		void (*Log)(const char* category, const char* fmt, ...);
		void* (*GetOrLoadModule)(const char* name);
		void* (*FindSignature)(void* module, const char* pattern, std::size_t offset);
		void* (*AddDetour)(const char* module, const char* pattern, void* detour, std::size_t offset);
		void* (*AddDetourExport)(const char* module, const char* exportName, void* detour);
		void (*AddInterfaceRewriter)(RewriteInterfacesFn fn);
		void (*AddClientClassRewriter)(RewriteClientClassesFn fn);
		void (*AddServerClassRewriter)(RewriteServerClassesFn fn);

#define DEFINE_MODULE_HOOK(RetType, Name, ...) RetType (*Name)(void (*callback)(__VA_ARGS__));
#include "defs/hook_events.h"
#undef DEFINE_MODULE_HOOK
	};
}

// Every module DLL must export:
//   extern "C" __declspec(dllexport) void ModuleMain(const gm_laboratory::ModuleAPI* api);
using ModuleMainFn = void (*)(const gm_laboratory::ModuleAPI* api);