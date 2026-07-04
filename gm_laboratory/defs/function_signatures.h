#pragma once

#if defined(_WIN32)
#if defined(PLATFORM_64BITS)
#define DECLARE_SIGSCAN_SIGNATURE_WIN32(name, signature) 
#define DECLARE_SIGSCAN_SIGNATURE_WIN64(name, signature) \
        inline constexpr const char* name##_SIG = signature;
#define DECLARE_SIGSCAN_SIGNATURE_LINUX32(name, signature) 
#define DECLARE_SIGSCAN_SIGNATURE_LINUX64(name, signature) 
#else
#define DECLARE_SIGSCAN_SIGNATURE_WIN32(name, signature) \
        inline constexpr const char* name##_SIG = "";
#define DECLARE_SIGSCAN_SIGNATURE_WIN64(name, signature) 
#define DECLARE_SIGSCAN_SIGNATURE_LINUX32(name, signature) 
#define DECLARE_SIGSCAN_SIGNATURE_LINUX64(name, signature) 
#endif
#else
#if defined(PLATFORM_64BITS)
#define DECLARE_SIGSCAN_SIGNATURE_WIN32(name, signature) 
#define DECLARE_SIGSCAN_SIGNATURE_WIN64(name, signature) 
#define DECLARE_SIGSCAN_SIGNATURE_LINUX32(name, signature) 
#define DECLARE_SIGSCAN_SIGNATURE_LINUX64(name, signature) \
        inline constexpr const char* name##_SIG = signature;
#else
#define DECLARE_SIGSCAN_SIGNATURE_WIN32(name, signature) 
#define DECLARE_SIGSCAN_SIGNATURE_WIN64(name, signature) 
#define DECLARE_SIGSCAN_SIGNATURE_LINUX32(name, signature) \
        inline constexpr const char* name##_SIG = "";
#define DECLARE_SIGSCAN_SIGNATURE_LINUX64(name, signature)
#endif
#endif

namespace gm_laboratory {
    DECLARE_SIGSCAN_SIGNATURE_WIN64(BootAppSystemGroup__Create, "48 8B C4 48 89 58 ? 48 89 78")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(CModAppSystemGroup__Main, "40 53 48 83 EC ? 80 B9 ? ? ? ? ? BB")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(_Host_RunFrame, "48 8B C4 48 89 58 ? 48 89 68 ? F3 0F 11 40")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(Host_Init, "48 89 5C 24 ? 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 0F 57 C0")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(luaL_loadbufferx, "4C 8B DC 53 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 48 8B F9")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(Cbuf_AddText, "48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 FF 15 ? ? ? ? 8B 15 ? ? ? ? 48 8B D8 3B C2 74 ? 45 33 C0 48 8D 0D ? ? ? ? 8B D3 FF 15 ? ? ? ? 84 C0 75 ? F3 90 45 33 C0 48 8D 0D ? ? ? ? 8B D3 FF 15 ? ? ? ? EB ? 90 8B 05 ? ? ? ? FF C0 89 05 ? ? ? ? 45 33 C9 48 8D 0D ? ? ? ? 48 8B D7 45 8D 41 ? E8 ? ? ? ? 84 C0 75 ? 48 8D 0D ? ? ? ? FF 15 ? ? ? ? 8B 05 ? ? ? ? FF C8 89 05 ? ? ? ? 8B 05 ? ? ? ? 85 C0 75 ? 33 D2 48 8D 0D ? ? ? ? 90 FF 15 ? ? ? ? 48 8B 5C 24 ? 48 83 C4 ? 5F C3 CC CC CC CC CC CC CC CC CC CC CC 40 53")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(Keys_HandleClientKey, "48 83 EC ? 44 8B 11")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(mesh_AdvanceVertex, "48 83 EC ? 48 8B D1 48 8B 49 ? 48 8B 01 FF 90 ? ? ? ? 48 83 3D ? ? ? ? ? 75 ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 01 FF 90 ? ? ? ? 33 C0 48 83 C4 ? C3 8B 15")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(NET_SendTo, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 48 8B 0D")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(Host_ReadConfiguration, "4C 8B DC 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 80 3D")
    DECLARE_SIGSCAN_SIGNATURE_WIN64(Host_WriteConfiguration, "40 55 53 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 85 C9")
}

#undef DECLARE_SIGSCAN_SIGNATURE