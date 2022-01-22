file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ASM.txt
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(ShISA_parser
                Driver.cpp)

add_subdirectory(tests)
