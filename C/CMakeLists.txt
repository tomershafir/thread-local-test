include (tl_add_test)

set (CMAKE_C_STANDARD 23)
set (CMAKE_C_STANDARD_REQUIRED ON)

if ((CMAKE_SYSTEM_NAME MATCHES "Linux"))
  set (OS_LINUX 1)
  add_definitions(-D OS_LINUX)
else()
  message (FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} is not supported")
endif ()

# By default, disable compiler optimizations 
# and keep debug info, to enable easier assembly
# inspection, being a testing repository.
# CMAKE_BUILD_TYPE of Debug implies -O0 -g for the compiler.
#
# Other build types are not supported.
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug")
endif ()

set (CMAKE_BUILD_TYPE 
  "${CMAKE_BUILD_TYPE}" 
  CACHE 
  STRING 
  "Choose the type of build." 
  FORCE)

set (DEFAULT_COMPILER_FLAGS -Wall -Wextra)

tl_add_test (c_tl_across_translation_units
  SOURCES 
    tl_across_translation_units/tl_across_translation_units_test.c
    tl_across_translation_units/tl_defs_impl.c
    tl_across_translation_units/tl_defs.h
  COMPILER_FLAGS
    ${DEFAULT_COMPILER_FLAGS}
    -pthread
)
