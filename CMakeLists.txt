cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("gm_laboratory")

if (MSVC)
  add_compile_options(/arch:AVX2)
else()
  add_compile_options(
    -msse
    -msse2
    -msse3
    -mssse3
    -msse4.1
    -msse4.2
    -mavx
    -mavx2
    -mfma
    -mpclmul
    -maes
    -mbmi
    -mbmi2
    -mpopcnt
    -mlzcnt
    -mf16c
  )
endif()


set_property(GLOBAL PROPERTY GML_MODULE_TARGETS "")

set(GMLAB_MODULE_DIR "$ENV{USERPROFILE}/gm_laboratory/modules")

function(add_gmlab_module name)
  add_library(${name} SHARED ${ARGN})
  target_link_libraries(${name} PRIVATE detours gm_laboratory source_sdk)
  target_include_directories(${name} PRIVATE "${CMAKE_SOURCE_DIR}/gm_laboratory")
  set_target_properties(${name} PROPERTIES
    CXX_STANDARD 20
    RUNTIME_OUTPUT_DIRECTORY "${GMLAB_MODULE_DIR}"
  )
  set_property(GLOBAL APPEND PROPERTY GMLAB_MODULE_TARGETS ${name})
endfunction()

add_subdirectory ("gm_laboratory")
add_subdirectory ("gm_laboratory_launcher")
add_subdirectory ("gm_laboratory_dedicated")
add_subdirectory ("custom_modules")
add_subdirectory ("example_modules")

get_property(gmlab_modules GLOBAL PROPERTY GMLAB_MODULE_TARGETS)
add_dependencies(gm_laboratory_launcher ${gmlab_modules})
add_dependencies(gm_laboratory_dedicated ${gmlab_modules})