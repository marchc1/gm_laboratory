# RenardDev/Detours (https://github.com/RenardDev/Detours) 
# Commit 2b7056562dda3a76fe0f4d8421dd70cad361f301

enable_language(ASM_MASM)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(DETOURS_ASM "interrupts64.asm")
else()
  set(DETOURS_ASM "interrupts32.asm")
endif()

add_library(detours STATIC
  "Detours.cpp"
  "Detours.h"
  "${DETOURS_ASM}"
)

target_include_directories(detours PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set_property(TARGET detours PROPERTY CXX_STANDARD 17)

# NOTE: Detours.h defines NOMINMAX and pulls in the NT types it needs
# Do not define NOMINMAX or WIN32_LEAN_AND_MEAN here: WIN32_LEAN_AND_MEAN 
# trims the headers that provide NTSTATUS, etc... which breaks everything
