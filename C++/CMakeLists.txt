set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

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

include (FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE # requires CMake 3.24
)
FetchContent_MakeAvailable (googletest)

function (_build_test name)
  cmake_parse_arguments (parsed_args
  ""
  ""
  "SOURCES;LIBRARIES"
  ${ARGN})
  
  if (NOT parsed_args_SOURCES)
    message (FATAL_ERROR "No source files provided for test ${name}.")
  endif ()

  set (target ${name}_test)

  add_executable (${target} ${parsed_args_SOURCES})
  
  target_compile_options (${target} PRIVATE -Wall -Wextra)
  
  if (parsed_args_LIBRARIES)
    target_link_libraries (${target} ${parsed_args_LIBRARIES})
  endif()
endfunction ()

_build_test (tl_across_translation_units
  SOURCES 
    tl_across_translation_units/tl_across_translation_units_test.cc
    tl_across_translation_units/tl_defs_impl.cc
    tl_across_translation_units/tl_defs.hh
  LIBRARIES
    gtest_main
)
