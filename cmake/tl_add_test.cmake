function (tl_add_test name)
  cmake_parse_arguments (parsed_args
  ""
  ""
  "SOURCES;COMPILER_FLAGS;LIBRARIES"
  ${ARGN})
  
  if (NOT parsed_args_SOURCES)
    message (FATAL_ERROR "No source files provided for test ${name}.")
  endif ()

  set (target ${name}_test)

  add_executable (${target} ${parsed_args_SOURCES})
  
  if (parsed_args_COMPILER_FLAGS)
    target_compile_options (${target} PRIVATE ${parsed_args_COMPILER_FLAGS})
  endif ()
  
  if (parsed_args_LIBRARIES)
    target_link_libraries (${target} ${parsed_args_LIBRARIES})
  endif()
endfunction ()
