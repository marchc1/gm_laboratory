add_subdirectory("../third_party/Detours" "../third_party/Detours")
add_subdirectory("../third_party/source-sdk" "../third_party/source-sdk")

add_library(gm_laboratory STATIC
  "defs/dll_names.h"
  "defs/function_signatures.h" 
  "defs/hook_events.h"
  "native/log.h"
  "native/log.cpp"
  "native/sigscan.h"
  "native/sigscan.cpp"
  "native/detour_manager.h"
  "native/detour_manager.cpp"
  "native/interface_registry.h"
  "native/interface_registry.cpp"
  "native/module_api.h"
  "native/modules.h"
  "native/modules.cpp"
  "moduleapi/hooks.h"
  "moduleapi/hooks.cpp"
  "moduleapi/hook_appframework.cpp"
  "moduleapi/hook_host.cpp"
  "moduleapi/hook_libraryload.cpp" "defs/sourcesdk_help.h" "defs/module_help.h")

target_include_directories(gm_laboratory PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(gm_laboratory PUBLIC detours)
target_link_libraries(gm_laboratory PUBLIC source_sdk)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET gm_laboratory PROPERTY CXX_STANDARD 20)
endif()