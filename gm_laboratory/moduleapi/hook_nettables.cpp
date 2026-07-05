#include "moduleapi/hooks.h"
#include "native/detour_manager.h"
#include "defs/dll_names.h"
#include "defs/function_signatures.h"
#include "client_class.h"
#include "server_class.h"
#include "util/connect_game_libraries.h"

#include "cdll_int.h"
#include "eiface.h"

// for cl: ClientDLL_InitRecvTableMgr will init
// for sv: SV_InitSendTables will init

namespace gm_laboratory {
	using ClientDLL_InitRecvTableMgr__Main = void (*)();
	using SV_InitSendTables__Main = void (*)(ServerClass*);

	static ClientDLL_InitRecvTableMgr__Main ClientDLL_InitRecvTableMgr_Original;
	static SV_InitSendTables__Main SV_InitSendTables_Original;

	static void ClientDLL_InitRecvTableMgr_Detour() {
		ClientClass* classes = g_ClientDLL->GetAllClasses();
		Hooks::PreInitClientClasses.Invoke(classes);
		ClientDLL_InitRecvTableMgr_Original();
		Hooks::PostInitClientClasses.Invoke(classes);
	}

	static void SV_InitSendTables_Detour(ServerClass* pClasses) {
		ServerClass* classes = g_ServerDLL->GetAllServerClasses();
		Hooks::PreInitServerClasses.Invoke(classes);
		SV_InitSendTables_Original(pClasses);
		Hooks::PostInitServerClasses.Invoke(classes);
	}

	struct PlugIntoNetTables : IImplementsDetours {
		void SetupWin64(DetourSetupContext& ctx) override {
			ClientDLL_InitRecvTableMgr_Original = reinterpret_cast<ClientDLL_InitRecvTableMgr__Main>(ctx.AddDetour(ENGINE_DLL, ClientDLL_InitRecvTableMgr_SIG, reinterpret_cast<void*>(&ClientDLL_InitRecvTableMgr_Detour)));
			SV_InitSendTables_Original = reinterpret_cast<SV_InitSendTables__Main>(ctx.AddDetour(ENGINE_DLL, SV_InitSendTables_SIG, reinterpret_cast<void*>(&SV_InitSendTables_Detour)));
		}
	};

	REGISTER_DETOUR(PlugIntoNetTables)
}
