#pragma once

#include <ShISA/Binary.hpp>

namespace benchmark {

enum Flags {
  ONLY_NOPS = 0,
  ONE_LOOP,
  ONE_LONG_LOOP,
  NESTED_LOOPS,
  FUNCTION_IN_LOOP,
  FUNCTION_WITH_NOPS_IN_LOOP,
  FIBONACCI,
  NUMBER_OF_FLAGS
};

auto getBenchmarkBinary(Flags flag = Flags::ONE_LOOP) -> shisa::Binary;

} // namespace benchmark
