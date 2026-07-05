add_subdirectory("../third_party/Detours" "../third_party/Detours")
add_subdirectory("../third_party/source-sdk" "../third_party/source-sdk")
add_subdirectory("../third_party/imgui" "../third_party/imgui")
add_subdirectory("../third_party/implot" "../third_party/implot")
add_subdirectory("../third_party/implot3d" "../third_party/implot3d")

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
  "native/net_class_editor.h"
  "native/net_class_editor.cpp"
  "native/vtable_editor.h"
  "native/vtable_editor.cpp"
  "native/overlay.h"
  "native/overlay.cpp"
  "native/laboratory/lab_app.h"
  "native/laboratory/laboratory.h"
  "native/laboratory/laboratory.cpp"
  "native/laboratory/apps/app_cvars.cpp"
  "native/module_api.h"
  "native/modules.h"
  "native/modules.cpp"
  "moduleapi/hooks.h"
  "moduleapi/hooks.cpp"
  "moduleapi/hook_appframework.cpp"
  "moduleapi/hook_host.cpp"
  "moduleapi/hook_libraryload.cpp"
  "defs/sourcesdk_help.h"
  "defs/module_help.h" 
  "moduleapi/hook_nettables.cpp" 
  "util/connect_game_libraries.h" 
  "util/connect_game_libraries.cpp" )

target_include_directories(gm_laboratory PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(gm_laboratory PUBLIC detours)
target_link_libraries(gm_laboratory PUBLIC source_sdk)
target_link_libraries(gm_laboratory PUBLIC imgui)
target_link_libraries(gm_laboratory PUBLIC implot)
target_link_libraries(gm_laboratory PUBLIC implot3d)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET gm_laboratory PROPERTY CXX_STANDARD 20)
endif()