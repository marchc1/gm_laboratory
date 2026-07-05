# RaphaeliT7/mini-source-sdk project ported from VPC -> CMake
# Commit d21e0bb2ce7ebd2d7be1ad9d5e3282adc17b360c
# Currently only win64; contributions welcome to change that.
# That being said, the future for Garry's Mod seems to be 64-bit exclusive,
# and I don't intend to target non-x86-64 branch Garry's Mod at the moment

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "source-sdk is configured for x64 only!")
endif()

set(SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(source_sdk_common INTERFACE)

target_include_directories(source_sdk_common INTERFACE
  "${SDK_ROOT}/common"
  "${SDK_ROOT}/public"
  "${SDK_ROOT}/public/tier0"
  "${SDK_ROOT}/public/tier1"
  "${SDK_ROOT}/public/mathlib"
)

target_compile_definitions(source_sdk_common INTERFACE
  WIN32 _WIN32 WIN64 _WIN64 PLATFORM_64BITS
  COMPILER_MSVC COMPILER_MSVC64
  # Garry's Mod game branch
  GAME_GMOD
  # No RAD Telemetry in this build.
  RAD_TELEMETRY_DISABLED
  # These libs are linked statically, so their headers must not dllimport.
  TIER1_STATIC_LIB TIER2_STATIC_LIB MATHLIB_LIB
  # Quiet the CRT and tolerate the tier0/vstdlib DLLs being built differently.
  _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE
  _ALLOW_RUNTIME_LIBRARY_MISMATCH
  _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH
  _ALLOW_MSC_VER_MISMATCH
)

# Engine core static libraries
target_link_libraries(source_sdk_common INTERFACE
  "${SDK_ROOT}/lib/public/x64/tier0.lib"
  "${SDK_ROOT}/lib/public/x64/vstdlib.lib"
)

# Build flags shared by some of the SDK libraries
function(source_sdk_configure target)
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 20)
  target_compile_options(${target} PRIVATE
    /fp:fast /GF /Gw /Zc:inline
    /wd4201 /wd4316 /wd4456 /wd4457 /wd4530 /wd4838 /wd5054 /wd5205
  )
endfunction()

# mathlib
file(GLOB MATHLIB_SOURCES CONFIGURE_DEPENDS "${SDK_ROOT}/mathlib/*.cpp")
add_library(mathlib STATIC ${MATHLIB_SOURCES})
target_link_libraries(mathlib PUBLIC source_sdk_common)
source_sdk_configure(mathlib)

# tier1 
add_library(tier1 STATIC
  "${SDK_ROOT}/tier1/bitbuf.cpp"
  "${SDK_ROOT}/tier1/newbitbuf.cpp"
  "${SDK_ROOT}/tier1/byteswap.cpp"
  "${SDK_ROOT}/tier1/characterset.cpp"
  "${SDK_ROOT}/tier1/checksum_crc.cpp"
  "${SDK_ROOT}/tier1/checksum_md5.cpp"
  "${SDK_ROOT}/tier1/checksum_sha1.cpp"
  "${SDK_ROOT}/tier1/commandbuffer.cpp"
  "${SDK_ROOT}/tier1/convar.cpp"
  "${SDK_ROOT}/tier1/datamanager.cpp"
  "${SDK_ROOT}/tier1/diff.cpp"
  "${SDK_ROOT}/tier1/generichash.cpp"
  "${SDK_ROOT}/tier1/ilocalize.cpp"
  "${SDK_ROOT}/tier1/interface.cpp"
  "${SDK_ROOT}/tier1/KeyValues.cpp"
  "${SDK_ROOT}/tier1/kvpacker.cpp"
  "${SDK_ROOT}/tier1/mempool.cpp"
  "${SDK_ROOT}/tier1/memstack.cpp"
  "${SDK_ROOT}/tier1/NetAdr.cpp"
  "${SDK_ROOT}/tier1/splitstring.cpp"
  "${SDK_ROOT}/tier1/processor_detect.cpp"
  "${SDK_ROOT}/tier1/rangecheckedvar.cpp"
  "${SDK_ROOT}/tier1/reliabletimer.cpp"
  "${SDK_ROOT}/tier1/stringpool.cpp"
  "${SDK_ROOT}/tier1/strtools.cpp"
  "${SDK_ROOT}/tier1/strtools_unicode.cpp"
  "${SDK_ROOT}/tier1/tier1.cpp"
  "${SDK_ROOT}/tier1/tokenreader.cpp"
  "${SDK_ROOT}/tier1/sparsematrix.cpp"
  "${SDK_ROOT}/tier1/uniqueid.cpp"
  "${SDK_ROOT}/tier1/utlbuffer.cpp"
  "${SDK_ROOT}/tier1/utlbufferutil.cpp"
  "${SDK_ROOT}/tier1/utlstring.cpp"
  "${SDK_ROOT}/tier1/utlsymbol.cpp"
  "${SDK_ROOT}/tier1/utlbinaryblock.cpp"
  "${SDK_ROOT}/tier1/snappy.cpp"
  "${SDK_ROOT}/tier1/snappy-sinksource.cpp"
  "${SDK_ROOT}/tier1/snappy-stubs-internal.cpp"
)

# processor_detect.cpp is compiled with exceptions upstream
set_source_files_properties("${SDK_ROOT}/tier1/processor_detect.cpp"
  PROPERTIES COMPILE_OPTIONS "/EHsc")
target_link_libraries(tier1 PUBLIC
  source_sdk_common
  Rpcrt4.lib
  ws2_32.lib
)
source_sdk_configure(tier1)

# tier2
add_library(tier2 STATIC "${SDK_ROOT}/tier2/tier2.cpp")
target_link_libraries(tier2 PUBLIC tier1 source_sdk_common)
source_sdk_configure(tier2)

# tier3
add_library(tier3 STATIC "${SDK_ROOT}/tier3/tier3.cpp")
target_link_libraries(tier3 PUBLIC tier1 source_sdk_common)
source_sdk_configure(tier3)

# Umbrella target for the whole Source SDK available
add_library(source_sdk INTERFACE)
target_link_libraries(source_sdk INTERFACE tier3 tier2 tier1 mathlib source_sdk_common)
