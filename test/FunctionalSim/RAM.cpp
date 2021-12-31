#include <FunctionalSim/RAMController.hpp>
#include <ShISA/Inst.hpp>
#include <exceptions.hpp>

#include <chrono>
#include <exception>
#include <iostream>
#include <limits>
#include <random>
#include <string>



using addr_t = uint16_t;
using cell_t = uint8_t;

using RAM = shisa::fsim::RAMBase<addr_t, cell_t>;

void testRAM() {
  constexpr auto test_name = __FUNCTION__;

  std::minstd_rand::result_type seed =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::minstd_rand rand{seed};

  RAM ram;

  bool   loopFinish = false;
  addr_t currAddr   = 0;
  while (!loopFinish) {
    const cell_t defaultVal = ram.read(currAddr);
    SHISA_CHECK_TEST(0 == defaultVal, std::string{test_name} +
                                          ": cell at address " +
                                          std::to_string(currAddr) +
                                          " not initialised with zero");
    const cell_t exampleData = rand();
    ram.write(currAddr, exampleData);
    const cell_t readData = ram.read(currAddr);
    SHISA_CHECK_TEST(readData == exampleData,
                     std::string{test_name} + ": cell at address " +
                         std::to_string(currAddr) +
                         " doesn't keep written value");

    if (std::numeric_limits<addr_t>::max() == currAddr++) {
      loopFinish = true;
    }
  }

#ifndef NDEBUG
  ram.dump(std::cout);
#endif
}

#if 0
template <template <typename Addr, typename Cell> class MemModel>
using RAM = shisa::fsim::RAMBase<addr_t, cell_t, MemModel>;

using FullRAM = RAM<shisa::fsim::FullMemModel>;
using LazyRAM = RAM<shisa::fsim::LazyMemModel>;

void testFullRAM() {
  constexpr auto test_name = __FUNCTION__;

  std::minstd_rand::result_type seed =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::minstd_rand rand{seed};

  FullRAM ram;

  bool   loopFinish = false;
  addr_t currAddr   = 0;
  while (!loopFinish) {
    const cell_t defaultVal = ram.read(currAddr);
    SHISA_CHECK_TEST(0 == defaultVal, std::string{test_name} +
                                          ": cell at address " +
                                          std::to_string(currAddr) +
                                          " not initialised with zero");
    const cell_t exampleData = rand();
    ram.write(currAddr, exampleData);
    const cell_t readData = ram.read(currAddr);
    SHISA_CHECK_TEST(readData == exampleData,
                     std::string{test_name} + ": cell at address " +
                         std::to_string(currAddr) +
                         " doesn't keep written value");

    if (std::numeric_limits<addr_t>::max() == currAddr++) {
      loopFinish = true;
    }
  }

  #ifndef NDEBUG
  ram.dump(std::cout);
  #endif
}

void testLazyRAM() {
  constexpr auto test_name = __FUNCTION__;

  std::minstd_rand::result_type seed =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::minstd_rand rand{seed};

  LazyRAM ram;

  bool   loopFinish = false;
  addr_t currAddr   = 0;
  while (!loopFinish) {
    const cell_t defaultVal = ram.read(currAddr);
    SHISA_CHECK_TEST(0 == defaultVal, std::string{test_name} +
                                          ": cell at address " +
                                          std::to_string(currAddr) +
                                          " not initialised with zero");
    const cell_t exampleData = rand();
    ram.write(currAddr, exampleData);
    const cell_t readData = ram.read(currAddr);
    SHISA_CHECK_TEST(readData == exampleData,
                     std::string{test_name} + ": cell at address " +
                         std::to_string(currAddr) +
                         " doesn't keep written value");

    if (std::numeric_limits<addr_t>::max() == currAddr++) {
      loopFinish = true;
    }
  }

  #ifndef NDEBUG
  ram.dump(std::cout);
  #endif
}
#endif



int main() {
  try {
#if 0
    testFullRAM();
    testLazyRAM();
#endif
    testRAM();
  } catch (const shisa::test::Exception &e) {
    std::cerr << __FILE__ ": test fail: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (const std::exception &e) {
    std::cerr << __FILE__ ": test fail: exception caught in main: " << e.what()
              << std::endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    std::cerr << __FILE__ ": test fail: unknown exception caught in main"
              << std::endl;
    exit(EXIT_FAILURE);
  }
}
