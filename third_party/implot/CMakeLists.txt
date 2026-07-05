# epezent/implot (https://github.com/epezent/implot)
# ImGui plotting extension. Shares the host ImGui context via ImPlot::SetImGuiContext.

add_library(implot STATIC
  "implot.cpp"
  "implot.h"
  "implot_internal.h"
  "implot_items.cpp"
  "implot_demo.cpp"
)

target_include_directories(implot PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(implot PUBLIC imgui)

set_property(TARGET implot PROPERTY CXX_STANDARD 17)
