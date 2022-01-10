#include <FunctionalSim/CPU.hpp>
#include <ShISA/Inst.hpp>
#include <exceptions.hpp>

#include <iostream>
#include <ranges>
#include <type_traits>



using Reg  = uint16_t;
using Addr = uint16_t;
using Cell = uint8_t;

using CPU = shisa::fsim::CpuBase<Reg, Addr, Cell, shisa::NREGS>;

using shisa::Binary;
using shisa::Inst;
using shisa::ISAModule;



// static asserts {{{
static_assert(std::is_same<shisa::Inst::RawInst, Reg>::value,
              "expected type of raw instruction is the same as register size");

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
// }}}



void testCPUInit() { // {{{
  constexpr auto test_name = __FUNCTION__;

  CPU cpu;

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

  for (auto r : std::views::iota(static_cast<size_t>(0), shisa::NREGS)) {
    const Reg regValue         = cpu.readReg(r);
    const Reg expectedRegValue = r == 1 ? 1 : 0;
    SHISA_CHECK_TEST(
        expectedRegValue == regValue,
        std::string{test_name} + ": r == " + std::to_string(regValue) +
            " but must be r == " + std::to_string(expectedRegValue));
  }
} // }}}

auto getTestBin() { // {{{
  std::vector insts = {
      Inst{0x1210}, // add r2  r1  r0
      Inst{0x1310}, // add r3  r1  r0
      Inst{0x1400}, // add r4  r0  r0
      Inst{0xa500}, // ld  r5  r0
      Inst{0x7645}, // cmp r6  r4  r5
      Inst{0x1711}, // add r7  r1  r1
      Inst{0xa877}, // ld  r8  r7
      Inst{0x9084}, // jtr r8  r4
      Inst{0x1930}, // add r9  r3  r0
      Inst{0x1332}, // add r3  r3  r2
      Inst{0x1290}, // add r2  r9  r0
      Inst{0x1441}, // add r4  r4  r1
      Inst{0x7645}, // cmp r6  r4  r5
      Inst{0xa811}, // ld  r8  r1
      Inst{0x1771}, // add r7  r7  r1
      Inst{0xaaa7}, // ld  r10 r7
      Inst{0xbaa3}, // st  r10 r3
  };

  ISAModule M{insts};

  std::vector<uint16_t> binData = {
      0x0005,
      0x0008,
      0x000f,
      0xffe2,
  };

  Binary bin{std::move(M), std::move(binData)};

  return std::move(bin);
} // }}}

void testPC() { // {{{
  constexpr auto test_name = __FUNCTION__;

  CPU cpu;

  {
    const Addr     PC         = cpu.getPC();
    constexpr Addr PCExpected = 0;
    SHISA_CHECK_TEST(PCExpected == PC, std::string{test_name} + ": PC set to " +
                                           std::to_string(PC) + " instead of " +
                                           std::to_string(PCExpected) +
                                           " before binary loading");
  }

  try {
    cpu.fetchNext();
    SHISA_CHECK_TEST(false, std::string{test_name} + ": PC set to " +
                                std::to_string(cpu.getPC()) +
                                " and an exception wasn't thrown when call "
                                "fetchNext() before binary loading");
  } catch (const shisa::ProgramEnd &e) {
    const Addr     PC         = cpu.getPC();
    constexpr Addr PCExpected = 0;
    SHISA_CHECK_TEST(PCExpected == PC,
                     std::string{test_name} + ": PC set to " +
                         std::to_string(PC) + " instead of " +
                         std::to_string(PCExpected) +
                         " when call fetchNext() before binary loading");
  }

  const Binary bin = getTestBin();
  cpu.loadBin(bin);

  {
    const Addr PC         = cpu.getPC();
    const Addr PCExpected = bin.nData() * CPU::cellsPerReg;
    SHISA_CHECK_TEST(PCExpected == PC, std::string{test_name} + ": PC set to " +
                                           std::to_string(PC) + " instead of " +
                                           std::to_string(PCExpected) +
                                           " after binary loading");
  }

  try {
    const Addr PC =
        bin.nInsts() * CPU::cellsPerInst + bin.nData() * CPU::cellsPerReg;
    cpu.setPC(PC);
    const Addr PCExpected =
        bin.nInsts() * CPU::cellsPerInst + bin.nData() * CPU::cellsPerReg;
    SHISA_CHECK_TEST(PCExpected == PC, std::string{test_name} + ": PC set to " +
                                           std::to_string(PC) + " instead of " +
                                           std::to_string(PCExpected));
  } catch (const shisa::ProgramEnd &e) {
    SHISA_CHECK_TEST(false, std::string{test_name} + ": PC set to " +
                                std::to_string(cpu.getPC()) +
                                " and an exception was thrown");
  }

  try {
    cpu.PCIncrement();
    SHISA_CHECK_TEST(false, std::string{test_name} + ": PC set to " +
                                std::to_string(cpu.getPC()) +
                                " and an exception wasn't thrown");
  } catch (const shisa::ProgramEnd &e) {
    const Addr PC = cpu.getPC();
    const Addr PCExpected =
        bin.nInsts() * CPU::cellsPerInst + bin.nData() * CPU::cellsPerReg;
    SHISA_CHECK_TEST(PCExpected == PC, std::string{test_name} + ": PC set to " +
                                           std::to_string(PC) + " instead of " +
                                           std::to_string(PCExpected));
  }
  {
    const Addr PCExpected = bin.nData() * CPU::cellsPerReg;
    cpu.setPC(PCExpected);
    const Addr PC = cpu.getPC();
    SHISA_CHECK_TEST(PCExpected == PC, std::string{test_name} + ": PC set to " +
                                           std::to_string(PC) + " instead of " +
                                           std::to_string(PCExpected));
  }

  for (auto i : std::views::iota(static_cast<size_t>(0), bin.nInsts())) {
    const Inst inst         = cpu.fetchNext();
    const Inst instExpected = bin.getISAModule().getInsts()[i];
    SHISA_CHECK_TEST(inst == instExpected,
                     std::string{test_name} + ": fetched instruction is " +
                         std::to_string(inst) + " instead of " +
                         std::to_string(instExpected) +
                         " with index = " + std::to_string(i));
  }

  try {
    cpu.fetchNext();
    SHISA_CHECK_TEST(false,
                     std::string{test_name} + ": PC set to " +
                         std::to_string(cpu.getPC()) +
                         " and an exception wasn't thrown when call "
                         "fetchNext() after last instruction was fetched");
  } catch (const shisa::ProgramEnd &e) {
    const Addr PC = cpu.getPC();
    const Addr PCExpected =
        bin.nInsts() * CPU::cellsPerInst + bin.nData() * CPU::cellsPerReg;
    SHISA_CHECK_TEST(
        PCExpected == PC,
        std::string{test_name} + ": PC set to " + std::to_string(PC) +
            " instead of " + std::to_string(PCExpected) +
            " when call fetchNext() after last instruction was fetched");
  }
} // }}}

void testSP() { // {{{
  constexpr auto test_name = __FUNCTION__;

  CPU cpu;

  {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = 0;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " before binary loading");
  }

  cpu.SPIncrement();

  {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = 1;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after SP increment");
  }

  cpu.SPDecrement();

  {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = 0;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after SP decrement");
  }

  try {
    cpu.SPDecrement();
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and an exception wasn't thrown when call "
                                "SPDecrement() when SP is zero");
  } catch (const shisa::StackUnderflow &e) {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = 0;
    SHISA_CHECK_TEST(SPExpected == SP,
                     std::string{test_name} + ": SP set to " +
                         std::to_string(SP) + " instead of " +
                         std::to_string(SPExpected) +
                         " after attempt to decrement it from zero value");
  }

  cpu.SPRegIncrement();

  {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = CPU::cellsPerReg;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after SP increment");
  }

  cpu.SPRegDecrement();

  {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = 0;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after SP decrement");
  }

  try {
    cpu.SPRegDecrement();
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and an exception wasn't thrown when call "
                                "SPDecrement() when SP is zero");
  } catch (const shisa::StackUnderflow &e) {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = 0;
    SHISA_CHECK_TEST(SPExpected == SP,
                     std::string{test_name} + ": SP set to " +
                         std::to_string(SP) + " instead of " +
                         std::to_string(SPExpected) +
                         " after attempt to decrement it from zero value");
  }

  constexpr Addr SPIncrement = 3;
  cpu.SPIncrementBy(SPIncrement);

  {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = SPIncrement;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after SP increment");
  }

  cpu.SPDecrementBy(SPIncrement);

  {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = 0;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after SP decrement");
  }

  try {
    cpu.SPDecrementBy(SPIncrement);
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and an exception wasn't thrown when call "
                                "SPDecrement() when SP is zero");
  } catch (const shisa::StackUnderflow &e) {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = 0;
    SHISA_CHECK_TEST(SPExpected == SP,
                     std::string{test_name} + ": SP set to " +
                         std::to_string(SP) + " instead of " +
                         std::to_string(SPExpected) +
                         " after attempt to decrement it from zero value");
  }


  try {
    cpu.loadRegFromStack(shisa::FIRST_WRITABLE_REG);
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and an exception wasn't thrown when call "
                                "loadFromStack() when stack is empty");
  } catch (const shisa::StackUnderflow &e) {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = 0;
    SHISA_CHECK_TEST(SPExpected == SP,
                     std::string{test_name} + ": SP set to " +
                         std::to_string(SP) + " instead of " +
                         std::to_string(SPExpected) +
                         " after attempt of load from empty stack");
  }

  try {
    for (auto i :
         std::views::iota(static_cast<size_t>(0), shisa::STACK_OFFSET)) {
      cpu.SPIncrement();
    }
    const Addr SP         = cpu.getSP();
    const Addr SPExpected = shisa::STACK_OFFSET;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected));
  } catch (const shisa::StackOverflow &e) {
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and stack overflow occured");
  }

  try {
    cpu.SPIncrement();
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and an exception wasn't thrown when "
                                "SPIncrement() must cause stack overflow");
  } catch (const shisa::StackOverflow &e) {
    const Addr     SP         = cpu.getSP();
    constexpr Addr SPExpected = shisa::STACK_OFFSET;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after overflow attempt");
  }

  const Binary bin = getTestBin();
  cpu.loadBin(bin);
  const Addr binEndAddr =
      bin.nInsts() * CPU::cellsPerInst + bin.nData() * CPU::cellsPerReg;

  {
    const Addr SP         = cpu.getSP();
    const Addr SPExpected = binEndAddr;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after binary load");
  }

  try {
    for (auto i : std::views::iota(static_cast<Addr>(0), shisa::STACK_OFFSET)) {
      cpu.SPIncrement();
    }
    const Addr SP         = cpu.getSP();
    const Addr SPExpected = binEndAddr + shisa::STACK_OFFSET;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected));
  } catch (const shisa::StackOverflow &e) {
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and stack overflow occured");
  }

  try {
    cpu.SPIncrement();
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and an exception wasn't thrown when "
                                "SPIncrement() must cause stack overflow");
  } catch (const shisa::StackOverflow &e) {
    const Addr SP         = cpu.getSP();
    const Addr SPExpected = binEndAddr + shisa::STACK_OFFSET;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after overflow attempt");
  }

  try {
    for (auto i : std::views::iota(static_cast<Addr>(0), shisa::STACK_OFFSET)) {
      cpu.SPDecrement();
    }
    const Addr SP         = cpu.getSP();
    const Addr SPExpected = binEndAddr;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected));
  } catch (const shisa::StackUnderflow &e) {
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and stack underflow occured");
  }

  try {
    cpu.SPDecrement();
    SHISA_CHECK_TEST(false, std::string{test_name} + ": SP set to " +
                                std::to_string(cpu.getSP()) +
                                " and an exception wasn't thrown when "
                                "SPIncrement() must cause stack overflow");
  } catch (const shisa::StackUnderflow &e) {
    const Addr SP         = cpu.getSP();
    const Addr SPExpected = binEndAddr;
    SHISA_CHECK_TEST(SPExpected == SP, std::string{test_name} + ": SP set to " +
                                           std::to_string(SP) + " instead of " +
                                           std::to_string(SPExpected) +
                                           " after underflow attempt");
  }

  Reg regFillVal = shisa::FIRST_WRITABLE_REG;
  for (Reg &reg : cpu.regs_range()) {
    reg = regFillVal++;
  }

  cpu.storeRegsOnStack();

  {
    const Addr SP = cpu.getSP();
    const Addr SPExpected =
        binEndAddr +
        (shisa::NREGS - shisa::FIRST_WRITABLE_REG) * CPU::cellsPerReg;
    SHISA_CHECK_TEST(SPExpected == SP,
                     std::string{test_name} + ": SP set to " +
                         std::to_string(SP) + " instead of " +
                         std::to_string(SPExpected) +
                         " after all registers were stored on stack");
  }

  cpu.loadRegsFromStack();

  {
    const Addr SP         = cpu.getSP();
    const Addr SPExpected = binEndAddr;
    SHISA_CHECK_TEST(SPExpected == SP,
                     std::string{test_name} + ": SP set to " +
                         std::to_string(SP) + " instead of " +
                         std::to_string(SPExpected) +
                         " after all registers were loaded from stack");
  }

  size_t regId = shisa::FIRST_WRITABLE_REG;
  for (const Reg reg : cpu.regs_range()) {
    Reg regExpected = regId;
    SHISA_CHECK_TEST(regExpected == reg,
                     std::string{test_name} + ": r" + std::to_string(regId) +
                         " set to " + std::to_string(reg) + " instead of " +
                         std::to_string(regExpected) +
                         " after all registers were loaded from stack");
    regId++;
  }
} // }}}

void testRF() { // {{{
  constexpr auto test_name = __FUNCTION__;

  CPU cpu;

  for (auto r : std::views::iota(static_cast<size_t>(0), shisa::NREGS)) {
    const Reg regValue         = cpu.readReg(r);
    const Reg expectedRegValue = r == 1 ? 1 : 0;
    SHISA_CHECK_TEST(
        expectedRegValue == regValue,
        std::string{test_name} + ": r == " + std::to_string(regValue) +
            " but must be r == " + std::to_string(expectedRegValue));
  }

  for (auto r : std::views::iota(static_cast<size_t>(0), shisa::NREGS)) {
    cpu.writeReg(r, static_cast<Reg>(r));
  }

  for (auto r : std::views::iota(static_cast<size_t>(0), shisa::NREGS)) {
    const Reg regValue         = cpu.readReg(r);
    const Reg expectedRegValue = r;
    SHISA_CHECK_TEST(
        expectedRegValue == regValue,
        std::string{test_name} + ": r == " + std::to_string(regValue) +
            " but must be r == " + std::to_string(expectedRegValue));
  }

  for (auto r : std::views::iota(static_cast<size_t>(0), shisa::NREGS)) {
    cpu.storeRegOnStack(r);
    cpu.loadRegFromStack(r);
    const Reg regValue         = cpu.readReg(r);
    const Reg expectedRegValue = r;
    SHISA_CHECK_TEST(
        expectedRegValue == regValue,
        std::string{test_name} + ": r == " + std::to_string(regValue) +
            " but must be r == " + std::to_string(expectedRegValue));
  }

  cpu.storeRegsOnStack();
  cpu.loadRegsFromStack();

  for (auto r : std::views::iota(static_cast<size_t>(0), shisa::NREGS)) {
    const Reg regValue         = cpu.readReg(r);
    const Reg expectedRegValue = r;
    SHISA_CHECK_TEST(
        expectedRegValue == regValue,
        std::string{test_name} + ": r == " + std::to_string(regValue) +
            " but must be r == " + std::to_string(expectedRegValue));
  }
} // }}}

void testRAM() { // {{{
  constexpr auto test_name = __FUNCTION__;

  CPU cpu;

  for (const Cell cell : cpu.ram_range()) {
    constexpr Cell expectedCell = 0;
    SHISA_CHECK_TEST(expectedCell == cell,
                     std::string{test_name} + ": cell contains " +
                         std::to_string(cell) + " but must constain " +
                         std::to_string(expectedCell));
  }

  const Binary bin     = getTestBin();
  const Addr   dataEnd = bin.nData() * CPU::cellsPerReg;
  const Addr   instEnd = bin.nInsts() * CPU::cellsPerInst + dataEnd;
  cpu.loadBin(bin);

  for (const auto addr : std::views::iota(static_cast<Addr>(0), dataEnd)) {
    const Cell cell = cpu.readFromRAM(addr);
    // sorry please sorry for this
    const Cell expectedCell = bin.getRawData()[addr / 2] >>
                              (((addr + 1) % 2) * sizeof(Cell) * CHAR_BIT);
    SHISA_CHECK_TEST(expectedCell == cell,
                     std::string{test_name} + ": cell with address " +
                         std::to_string(addr) + " contains " +
                         std::to_string(cell) + " but must constain " +
                         std::to_string(expectedCell));
  }

  for (const auto addr : std::views::iota(dataEnd, instEnd)) {
    const Cell cell = cpu.readFromRAM(addr);
    // sorry please sorry for this too
    const Cell expectedCell =
        bin.getISAModule().getInsts()[(addr - dataEnd) / 2] >>
        (((addr - dataEnd + 1) % 2) * sizeof(Cell) * CHAR_BIT);
    SHISA_CHECK_TEST(expectedCell == cell,
                     std::string{test_name} + ": cell with address " +
                         std::to_string(addr) + " contains " +
                         std::to_string(cell) + " but must constain " +
                         std::to_string(expectedCell));
  }

  for (auto r : std::views::iota(static_cast<size_t>(0), shisa::NREGS)) {
    cpu.writeReg(r, static_cast<Reg>(r));
  }

  cpu.storeRegsOnStack();

  for (const Reg r :
       std::views::iota(shisa::FIRST_WRITABLE_REG, shisa::NREGS)) {
    const Addr addr =
        instEnd + (r - shisa::FIRST_WRITABLE_REG) * CPU::cellsPerReg;
    cpu.readRegFromRAM(addr, r);
    const Reg reg         = cpu.readReg(r);
    const Reg expectedReg = r;
    SHISA_CHECK_TEST(expectedReg == reg,
                     std::string{test_name} + ": cells at addreses " +
                         std::to_string(addr) + "-" + std::to_string(addr + 1) +
                         " contains " + std::to_string(reg) +
                         " but must constain " + std::to_string(expectedReg));
  }
} // }}}



int main() {
  try {
    testCPUInit();
    testPC();
    testSP();
    testRF();
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
