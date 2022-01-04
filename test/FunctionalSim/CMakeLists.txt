set(TEST_LIST CPU RegisterFile RAM RAMController SwitchedSim)

if(BUILD_TESTING)
  foreach(TEST IN LISTS TEST_LIST)
    add_executable(${TEST} "${CMAKE_CURRENT_SOURCE_DIR}/${TEST}.cpp")
    add_test(NAME "FunctionalSim-${TEST}" COMMAND ${TEST})
  endforeach()
endif()
