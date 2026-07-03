#include <windows.h>

#include "moduleapi/hooks.h"
#include "native/detour_manager.h"

namespace gm_laboratory {

using LoadLibraryExAFn = HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD);

static LoadLibraryExAFn OriginalLoadLibraryExA;

static HMODULE WINAPI Hooked_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE handle, DWORD flags) {
	Hooks::PreLoadLibrary.Invoke(lpLibFileName);
	HMODULE ret = OriginalLoadLibraryExA(lpLibFileName, handle, flags);
	Hooks::PostLoadLibrary.Invoke(lpLibFileName, reinterpret_cast<void*>(ret));
	return ret;
}

struct OSLibraryLoadFunctions : IImplementsDetours {
	void SetupWin64(DetourSetupContext& ctx) override {
		OriginalLoadLibraryExA = reinterpret_cast<LoadLibraryExAFn>(ctx.AddDetourExport("kernelbase.dll", "LoadLibraryExA", reinterpret_cast<void*>(&Hooked_LoadLibraryExA)));
	}
};

REGISTER_DETOUR(OSLibraryLoadFunctions)

}
