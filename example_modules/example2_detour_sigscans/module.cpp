#include "native/module_api.h"
#include "defs/module_help.h"
#include "defs/dll_names.h"
#include "defs/function_signatures.h"

#include <cstddef>

using namespace gm_laboratory;

using lua_State = void;
using luaL_loadbufferx_t = int (*)(lua_State* L, const char* buff, std::size_t sz, const char* name, const char* mode);

MODULE_START()

static luaL_loadbufferx_t g_originalLoadBuffer = nullptr;

static int luaL_loadbufferx_Detour(lua_State* L, const char* buff, std::size_t sz, const char* name, const char* mode) {
	g_api->Log("example2", "luaL_loadbufferx: compiling chunk '%s' (%zu bytes)\n", name ? name : "<null>", sz);
	return g_originalLoadBuffer(L, buff, sz, name, mode);
}

MODULE_MAIN() {
	g_api->Log("example2", "Loaded Example Module #2: Detouring via Signature Scanning (abi %u)\n", api->abiVersion);

	g_originalLoadBuffer = reinterpret_cast<luaL_loadbufferx_t>(
		g_api->AddDetour(LUASHARED_DLL, luaL_loadbufferx_SIG, reinterpret_cast<void*>(&luaL_loadbufferx_Detour), 0));

	if (g_originalLoadBuffer)
		g_api->Log("example2", "hooked luaL_loadbufferx in %s\n", LUASHARED_DLL);
	else
		g_api->Log("example2", "failed to hook luaL_loadbufferx (signature miss or module not loaded)\n");
} MODULE_END()
