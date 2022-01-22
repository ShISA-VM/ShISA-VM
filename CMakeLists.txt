cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_CXX_STANDARD 20)

project(ShISA)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." ON)

if(${FORCE_COLORED_OUTPUT})
  message("-- Force colored output enabled")

  if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
     add_compile_options(-fdiagnostics-color=always)
  elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
     add_compile_options(-fcolor-diagnostics)
  endif()
endif()

include_directories(include)
add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(ShISAParser)

enable_testing()

set(BUILD_TESTING ON)

add_subdirectory(test)

