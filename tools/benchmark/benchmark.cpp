#include "benchmark.hpp"

#include <FunctionalSim/PredecodedSim.hpp>
#include <FunctionalSim/PredecodedSubroutinedSim.hpp>
#include <FunctionalSim/SubroutinedSim.hpp>
#include <FunctionalSim/SwitchedSim.hpp>
#include <FunctionalSim/isSim.hpp>
#include <ShISA/Binary.hpp>

#include <cassert>
#include <chrono>
#include <ranges>

using PredecodedSim            = shisa::fsim::PredecodedSim<>;
using PredecodedSubroutinedSim = shisa::fsim::PredecodedSubroutinedSim<>;
using SubroutinedSim           = shisa::fsim::SubroutinedSim<>;
using SwitchedSim              = shisa::fsim::SwitchedSim<>;



namespace benchmark {

auto getFlagName(Flags flag) -> std::string {
  switch (flag) {
  case benchmark::ONLY_NOPS:
    return "Only nops:";
  case benchmark::ONE_LOOP:
    return "One loop:";
  case benchmark::ONE_LONG_LOOP:
    return "One long loop:";
  case benchmark::NESTED_LOOPS:
    return "Nested loops:";
  case benchmark::FUNCTION_IN_LOOP:
    return "Function in loop:";
  case benchmark::FUNCTION_WITH_NOPS_IN_LOOP:
    return "Function with nops in loop:";
  case benchmark::FIBONACCI:
    return "Fibonacci:";
  default:
    return "Unknown flag:";
  }
}

template <shisa::fsim::isSim Sim_>
class Benchmark {
public:
  using Clock    = std::chrono::system_clock;
  using Duration = std::chrono::duration<double>;
  using Time     = std::chrono::time_point<Clock>;

  using Sim = Sim_;

private:
  std::array<bool, Flags::NUMBER_OF_FLAGS>     measured{false};
  std::array<Duration, Flags::NUMBER_OF_FLAGS> durations;

public:
  void run(Flags flag) {
    Time start = Clock::now();
    Sim_ sim{std::move(getBenchmarkBinary(flag))};
    sim.executeAll();
    Time end        = Clock::now();
    durations[flag] = end - start;
    measured[flag]  = true;
  }

  auto getDuration(Flags flag) -> Duration {
    if (!measured[flag]) {
      run(flag);
    }

    return durations[flag];
  }
};

#if 0
// TODO: make benchmark for several sims that recursively goes through single-sim benchmarks in tuple
template <shisa::fsim::isSim... Sims>
class BenchmarkMultiple {
  std::tuple<Benchmark<Sims>...> benchmarks;

public:
};
#endif

} // namespace benchmark



int main() {
  using benchmark::Benchmark;
  Benchmark<PredecodedSim>            b1{};
  Benchmark<PredecodedSubroutinedSim> b2{};
  Benchmark<SubroutinedSim>           b3{};
  Benchmark<SwitchedSim>              b4{};

  auto processBenchmark = [](auto b, benchmark::Flags flag) {
    std::cout << std::setw(25) << std::left
              << shisa::fsim::getSimName<typename decltype(b)::Sim>() << " "
              << std::fixed << b.getDuration(flag).count() << '\n';
  };

  for (const benchmark::Flags flag : {
           benchmark::FIBONACCI,
           benchmark::ONLY_NOPS,
           benchmark::ONE_LOOP,
           benchmark::FUNCTION_IN_LOOP,
           benchmark::NESTED_LOOPS,
           benchmark::ONE_LONG_LOOP,
           benchmark::FUNCTION_WITH_NOPS_IN_LOOP,
       }) {
    std::cout << benchmark::getFlagName(flag) << '\n';
    processBenchmark(b1, flag);
    processBenchmark(b2, flag);
    processBenchmark(b3, flag);
    processBenchmark(b4, flag);
    std::cout << std::endl;
  }
}
