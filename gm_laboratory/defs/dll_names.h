#pragma once


#define DLL_EXTENSION_DEF ".dll"
#define DECLARE_DLL_NAME(prefix, base) \
    inline constexpr const char* prefix##_DLL = base DLL_EXTENSION_DEF;

namespace gm_laboratory {
	DECLARE_DLL_NAME(CLIENT,				"client")
	DECLARE_DLL_NAME(DATACACHE,			"datacache")
	DECLARE_DLL_NAME(DEDICATED,			"dedicated")
	DECLARE_DLL_NAME(ENGINE,				"engine")
	DECLARE_DLL_NAME(FILESYSTEM,			"filesystem_stdio")
	DECLARE_DLL_NAME(GAMEUI,				"GameUI")
	DECLARE_DLL_NAME(GMOD_AUDIO,			"gmod_audio")
	DECLARE_DLL_NAME(INPUTSYSTEM,		"inputsystem")
	DECLARE_DLL_NAME(LAUNCHER,			"launcher")
	DECLARE_DLL_NAME(LUASHARED,			"lua_shared")
	DECLARE_DLL_NAME(MATERIALSYSTEM,		"materialsystem")
	DECLARE_DLL_NAME(MENUSYSTEM,			"menusystem")
	DECLARE_DLL_NAME(SERVER,				"server")
	DECLARE_DLL_NAME(SERVERBROWSER,		"ServerBrowser")
	DECLARE_DLL_NAME(SHADERAPI,			"shaderapidx9")
	DECLARE_DLL_NAME(SOUNDSSYSTEM,		"soundsystem")
	DECLARE_DLL_NAME(STUDIORENDER,		"studiorender")
	DECLARE_DLL_NAME(TIER0,				"tier0")
	DECLARE_DLL_NAME(VGUI2,				"vgui2")
	DECLARE_DLL_NAME(VGUIMATSURFACE,		"vguimatsurface")
	DECLARE_DLL_NAME(VPHYSICS,			"vphysics")
	DECLARE_DLL_NAME(VSTDLIB,			"vstdlib")
} 

#undef DECLARE_DLL_NAME
#undef DLL_EXTENSION_DEF