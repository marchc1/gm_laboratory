#ifdef DEFINE_MODULE_HOOK
/// <summary> Runs right before Host_Init </summary>
DEFINE_MODULE_HOOK(void, PreHostInit, bool)

/// <summary> Runs right after Host_Init </summary>
DEFINE_MODULE_HOOK(void, PostHostInit, bool)

/// <summary> Runs right before _Host_RunFrame </summary>
DEFINE_MODULE_HOOK(void, PreHostRunFrame, double)

/// <summary> Runs right after _Host_RunFrame </summary>
DEFINE_MODULE_HOOK(void, PostHostRunFrame, double)

/// <summary> Called before a dynamic library is loaded by the OS, with the module name passed as an argument </summary>
DEFINE_MODULE_HOOK(void, PreLoadLibrary, const char*)

/// <summary> Called after a dynamic library has been loaded by the OS, with the module name and module address passed as arguments </summary>
DEFINE_MODULE_HOOK(void, PostLoadLibrary, const char*, void*)

/// <summary> Called very early on in the launcher procedure; good for doing low level modifications </summary>
DEFINE_MODULE_HOOK(void, PreLauncherMain)

/// <summary> Runs before launcher.dll::BootAppSystemGroup::Create (before engine is bootstrapped) </summary>
DEFINE_MODULE_HOOK(void, PreBootAppSystemCreate)
/// <summary> Runs after launcher.dll::BootAppSystemGroup::Create (before engine is bootstrapped) </summary>
DEFINE_MODULE_HOOK(void, PostBootAppSystemCreate)

/// <summary> Called before eng->Load roughly </summary>
DEFINE_MODULE_HOOK(void, PreModAppSystemMain)

#endif // If you define more hooks, define them after the last hook!!