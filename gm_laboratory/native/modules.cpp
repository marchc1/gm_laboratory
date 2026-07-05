#include "native/modules.h"

#include <cstdio>
#include <vector>

#include <windows.h>

#include "KeyValues.h"

#include "native/detour_manager.h"
#include "native/log.h"
#include "native/module_api.h"
#include "native/sigscan.h"
#include "moduleapi/hooks.h"

class ClientClass;
class ServerClass;

namespace gm_laboratory {

	static void* ApiAddDetour(const char* module, const char* pattern, void* detour, std::size_t offset) {
		DetourSetupContext ctx;
		return ctx.AddDetour(module, pattern, detour, offset);
	}

	static void* ApiAddDetourExport(const char* module, const char* exportName, void* detour) {
		DetourSetupContext ctx;
		return ctx.AddDetourExport(module, exportName, detour);
	}

#define DEFINE_MODULE_HOOK(RetType, Name, ...) \
    static void Api##Name(RetType (*callback)(__VA_ARGS__)) { \
        Hooks::Name += std::function<RetType(__VA_ARGS__)>(callback); \
    }
#include "defs/hook_events.h"
#undef DEFINE_MODULE_HOOK

	static const ModuleAPI& GetModuleAPI() {
		static ModuleAPI api = {
			MODULE_ABI_VERSION,
			&Log,
			&GetOrLoadModule,
			&FindSignature,
			&ApiAddDetour,
			&ApiAddDetourExport,
			&Hooks::AddInterfaceRewriter,
			&Hooks::AddClientClassRewriter,
			&Hooks::AddServerClassRewriter,

	#define DEFINE_MODULE_HOOK(RetType, Name, ...) &Api##Name,
	#include "defs/hook_events.h"
	#undef DEFINE_MODULE_HOOK
		};
		return api;
	}

	bool Modules::Load(const char* name) {
		HMODULE module = LoadLibraryExA(name, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (!module) {
			Log("modules", "Failed to load module %s\n", name);
			return false;
		}

		auto entry = reinterpret_cast<ModuleMainFn>(GetProcAddress(module, "ModuleMain"));
		if (!entry) {
			Log("modules", "Module %s has no ModuleMain export\n", name);
			return false;
		}

		entry(&GetModuleAPI());
		Log("modules", "Loaded module %s\n", name);
		return true;
	}

	static KeyValues* LoadBlacklist(const char* folder) {
		char path[MAX_PATH];
		_snprintf_s(path, _TRUNCATE, "%s\\blacklist.cfg", folder);

		FILE* file = fopen(path, "rb");
		if (!file)
			return nullptr;

		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		fseek(file, 0, SEEK_SET);

		std::vector<char> buffer(size + 1);
		if (size > 0)
			fread(buffer.data(), 1, size, file);
		buffer[size] = '\0';
		fclose(file);

		KeyValues* blacklist = new KeyValues("blacklist");
		if (!blacklist->LoadFromBuffer(path, buffer.data())) {
			Log("modules", "Failed to parse blacklist %s\n", path);
			blacklist->deleteThis();
			return nullptr;
		}

		Log("modules", "Loaded blacklist %s\n", path);
		return blacklist;
	}

	static bool IsBlacklisted(KeyValues* blacklist, const char* fileName) {
		return blacklist && blacklist->GetInt(fileName, 0) != 0;
	}

	int Modules::LoadFolder(const char* folder) {
		KeyValues* blacklist = LoadBlacklist(folder);

		char search[MAX_PATH];
		_snprintf_s(search, _TRUNCATE, "%s\\*.dll", folder);

		WIN32_FIND_DATAA fd;
		HANDLE find = FindFirstFileA(search, &fd);
		if (find == INVALID_HANDLE_VALUE) {
			Log("modules", "No modules found in %s\n", folder);
			if (blacklist)
				blacklist->deleteThis();
			return 0;
		}

		int loaded = 0;
		do {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			char filename[MAX_PATH];
			V_StripExtension(fd.cFileName, filename, sizeof(filename));
			if (IsBlacklisted(blacklist, filename)) {
				Log("modules", "Skipping blacklisted module %s\n", fd.cFileName);
				continue;
			}

			char path[MAX_PATH];
			_snprintf_s(path, _TRUNCATE, "%s\\%s", folder, fd.cFileName);
			if (Load(path))
				++loaded;
		} while (FindNextFileA(find, &fd));

		FindClose(find);

		if (blacklist)
			blacklist->deleteThis();

		return loaded;
	}

}
