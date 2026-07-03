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
}

#undef DECLARE_SIGSCAN_SIGNATURE