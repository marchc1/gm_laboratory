#include "util/connect_game_libraries.h"
#include <interface.h>
#include "tier2/tier2.h"
#include "filesystem.h"
#include "defs/dll_names.h"

#include "cdll_int.h"
#include "eiface.h"

namespace gm_laboratory {
	IBaseClientDLL* g_ClientDLL = nullptr;
	IVEngineClient013* g_EngineCL = nullptr;
	IServerGameDLL* g_ServerDLL = nullptr;
	IVEngineServer021* g_EngineSV = nullptr;
	
	template <typename T>
	static T* loadModule(const char* module, const char* name) {
		T* t = nullptr;
		CSysModule* systemModule = g_pFullFileSystem->LoadModule(module, "GAMEBIN", false);
		if (systemModule) {
			CreateInterfaceFn factory = Sys_GetFactory(systemModule);
			if (factory)
				t = (T*)factory(name, NULL);
		}
		return t;
	}

	void ConnectGameLibraries() {
		g_ClientDLL = loadModule<IBaseClientDLL>(CLIENT_DLL, CLIENT_DLL_INTERFACE_VERSION);
		g_ServerDLL = loadModule<IServerGameDLL>(SERVER_DLL, INTERFACEVERSION_SERVERGAMEDLL);
		g_EngineCL = loadModule<IVEngineClient013>(ENGINE_DLL, VENGINE_CLIENT_INTERFACE_VERSION);
		g_EngineSV = loadModule<IVEngineServer021>(ENGINE_DLL, INTERFACEVERSION_VENGINESERVER);
	}
}