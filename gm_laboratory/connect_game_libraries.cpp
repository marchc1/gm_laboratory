#include "connect_game_libraries.h"
#include <interface.h>
#include "tier2/tier2.h"
#include "filesystem.h"
#include "defs/dll_names.h"

#include "cdll_int.h"
#include "eiface.h"

namespace gm_laboratory {
	IBaseClientDLL* g_ClientDLL = nullptr;
	IServerGameDLL* g_ServerDLL = nullptr;

	void ConnectGameLibraries() {
		CSysModule* clientDLLModule = g_pFullFileSystem->LoadModule(CLIENT_DLL, "GAMEBIN", false);
		if (clientDLLModule) {
			CreateInterfaceFn clientFactory = Sys_GetFactory(clientDLLModule);
			if(clientFactory)
				g_ClientDLL = (IBaseClientDLL*)clientFactory(CLIENT_DLL_INTERFACE_VERSION, NULL);
		}

		CSysModule* serverDLLModule = g_pFullFileSystem->LoadModule(SERVER_DLL, "GAMEBIN", false);
		if (serverDLLModule) {
			CreateInterfaceFn serverFactory = Sys_GetFactory(serverDLLModule);
			if (serverFactory) 
				g_ServerDLL = (IServerGameDLL*)serverFactory(INTERFACEVERSION_SERVERGAMEDLL, NULL);
		}
	}
}