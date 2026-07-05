# ocornut/imgui (https://github.com/ocornut/imgui) - docking branch
# Garry's Mod x64 renders through DirectX 9 (shaderapidx9), so we build
# the Win32 platform backend + the DX9 renderer backend.

add_library(imgui STATIC
  "imgui.cpp"
  "imgui.h"
  "imgui_demo.cpp"
  "imgui_draw.cpp"
  "imgui_tables.cpp"
  "imgui_widgets.cpp"
  "imgui_internal.h"
  "imconfig.h"
  # Backends
  "backends/imgui_impl_win32.cpp"
  "backends/imgui_impl_win32.h"
  "backends/imgui_impl_dx9.cpp"
  "backends/imgui_impl_dx9.h"
)

target_include_directories(imgui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/backends"
)

target_link_libraries(imgui PUBLIC d3d9)

set_property(TARGET imgui PROPERTY CXX_STANDARD 17)
