set(BENCHMARK_SOURCES benchmark.cpp benchmarkBinary.cpp)

add_executable(benchmark ${BENCHMARK_SOURCES})

install(TARGETS benchmark
    DESTINATION bin
    COMPONENT benchmark
    )
