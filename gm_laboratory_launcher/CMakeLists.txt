file(GLOB SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")
add_executable (gm_laboratory_launcher ${SOURCES})

target_link_libraries(gm_laboratory_launcher PRIVATE gm_laboratory)

target_link_options(gm_laboratory_launcher PRIVATE "/WHOLEARCHIVE:gm_laboratory")
set_property(TARGET gm_laboratory_launcher PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$ENV{GARRYSMOD_INSTALL_DIR}/bin/win64")
set_target_properties(gm_laboratory_launcher PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$ENV{GARRYSMOD_INSTALL_DIR}/bin/win64"
)
set_target_properties(gm_laboratory_launcher PROPERTIES WIN32_EXECUTABLE TRUE)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET gm_laboratory_launcher PROPERTY CXX_STANDARD 20)
endif()