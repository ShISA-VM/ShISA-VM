#include <FunctionalSim/FunctionalSim.hpp>
#include <ShISA/Inst.hpp>
#include <exceptions.hpp>

#include <iostream>



using Reg  = uint16_t;
using Addr = uint16_t;
using Cell = uint8_t;

using CPU = shisa::fsim::CpuBase<Reg, Addr, Cell, shisa::NREGS>;

using shisa::Inst;



constexpr bool regEndAlignedExpected = true;
static_assert(
    regEndAlignedExpected == (sizeof(Reg) % sizeof(Cell) == 0),
    "expected register size with aligned end using this type of cell");
static_assert(CPU::regEndAligned == regEndAlignedExpected,
              "Unexpected register size end align");

constexpr size_t cellsPerRegExpected = 2;
static_assert(cellsPerRegExpected == (sizeof(Reg) / sizeof(Cell)),
              "expected another number of cells per register");
static_assert(CPU::cellsPerReg == cellsPerRegExpected,
              "Unexpected number of cells per register");

constexpr bool instEndAlignedExpected = true;
static_assert(
    instEndAlignedExpected == (sizeof(Inst) % sizeof(Cell) == 0),
    "expected instruction size with aligned end using this type of cell");
static_assert(CPU::instEndAligned == instEndAlignedExpected,
              "Unexpected instruction size end align");

constexpr size_t cellsPerInstExpected = 2;
static_assert(cellsPerInstExpected == (sizeof(Inst) / sizeof(Cell)),
              "expected another number of cells per instruction");
static_assert(CPU::cellsPerInst == cellsPerInstExpected,
              "Unexpected number of cells per instruction");



void testCPU() {
  constexpr auto test_name = __FUNCTION__;

  CPU cpu;

#ifndef NDEBUG
  cpu.dump(std::clog);
#endif

  const Addr     PC         = cpu.getPC();
  constexpr Addr PCExpected = 0;
  SHISA_CHECK_TEST(PCExpected == PC, std::string{test_name} + ": PC set to " +
                                         std::to_string(PC) + " instead of " +
                                         std::to_string(PCExpected) +
                                         " before binary loading");

  const Addr     SP         = cpu.getSP();
  constexpr Addr SPExpected = 0;
  SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                         std::to_string(SP) + " instead of " +
                                         std::to_string(SPExpected) +
                                         " before binary loading");
}



int main() {
  try {
    testCPU();
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
