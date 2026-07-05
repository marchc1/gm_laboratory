# brenocq/implot3d (https://github.com/brenocq/implot3d)
# 3D plotting extension for ImGui. Uses the host ImGui context set via ImGui::SetCurrentContext.

add_library(implot3d STATIC
  "implot3d.cpp"
  "implot3d.h"
  "implot3d_internal.h"
  "implot3d_items.cpp"
  "implot3d_meshes.cpp"
  "implot3d_demo.cpp"
)

target_include_directories(implot3d PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(implot3d PUBLIC imgui)

set_property(TARGET implot3d PROPERTY CXX_STANDARD 17)
