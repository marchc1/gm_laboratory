# gm_laboratory

A native modding experiments toolkit for Garry's Mod (only for x86-64 branch at the moment)

To use it, you will need to define GARRYSMOD_INSTALL_DIR in your environment variables. Just set this to the GarrysMod path with no trailing slash.

When built, it will output gm_laboratory_launcher.exe to your GARRYSMOD_INSTALL_DIR/bin/win64 folder, with handling logic that loads your own DLLs before the engine starts

## How it works

`gm_laboratory_launcher.exe` drops in next to gmod.exe. (you can fully replace it if you are that confident, but note that Chromium currently does not work here.). On startup it:

1. Loads every DLL in `%USERPROFILE%\gm_laboratory\modules`
2. Calls each module's exported `ModuleMain(const ModuleAPI*)`
3. Hands off to the real `launcher.dll`

Modules get a `ModuleAPI` struct with function pointers for logging, signature scanning, detouring, interface rewriting, and subscribing to a handful of engine lifecycle hooks (see `gm_laboratory/defs/hook_events.h`). Expect this to not be perfectly ABI compatible in the future as you upgrade versions, or explicitly check abiVersion (YYYYmmDDrr year month day revision is the format)

## Building

My personal setup is:
- Visual Studio 2026
- CMake 3.10 + Ninja

Built modules land in `%USERPROFILE%\gm_laboratory\modules` automatically (see `add_gmlab_module` in the root `CMakeLists.txt`). 
(this might move later to AppData... probably a better place...)

## Writing a module

Drop a folder under `custom_modules/` with a `CMakeLists.txt`:

```cmake
add_gmlab_module(my_module "module.cpp")
```

and a `module.cpp`:

```cpp
#include "native/module_api.h"
#include "defs/module_help.h"

using namespace gm_laboratory;

MODULE_START()
MODULE_MAIN()
g_api->Log("my_module", "hello from my module\n");
MODULE_END()
```

`custom_modules/` and `example_modules/` are both scanned automatically by CMake, so any subfolder with a `CMakeLists.txt` gets built

Some example modules have been provided, YMMV as I am still working on them