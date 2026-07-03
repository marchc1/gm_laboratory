#include "moduleapi/hooks.h"
#include "native/detour_manager.h"
#include "defs/dll_names.h"
#include "defs/function_signatures.h"

namespace gm_laboratory {
	using _Host_RunFrame__Main = void (*)(double);

	static _Host_RunFrame__Main _Host_RunFrame_Original;

	static void _Host_RunFrame_Detour(double time) {
		Hooks::PreHostRunFrame.Invoke(time);
		_Host_RunFrame_Original(time);
		Hooks::PostHostRunFrame.Invoke(time);
	}

	struct PlugIntoHost : IImplementsDetours {
		void SetupWin64(DetourSetupContext& ctx) override {
			_Host_RunFrame_Original = reinterpret_cast<_Host_RunFrame__Main>(ctx.AddDetour(ENGINE_DLL, _Host_RunFrame_SIG, reinterpret_cast<void*>(&_Host_RunFrame_Detour)));
		}
	};

	REGISTER_DETOUR(PlugIntoHost)

}
