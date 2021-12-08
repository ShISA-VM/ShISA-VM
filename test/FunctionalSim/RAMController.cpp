#include <FunctionalSim/FunctionalSim.hpp>
#include <ShISA/Inst.hpp>
#include <exceptions.hpp>

#include <iostream>
#include <limits>
#include <string>
#include <vector>



using Addr = uint16_t;
using Cell = uint8_t;

using RAMController = shisa::fsim::RAMControllerBase<Addr, Cell>;

using shisa::Binary;
using shisa::Inst;
using shisa::ISAModule;

constexpr bool instEndAlignedExpected =
    (sizeof(ISAModule::RawInst) % sizeof(Cell)) == 0;
constexpr size_t cellsPerInstExpected =
    sizeof(ISAModule::RawInst) / sizeof(Cell) +
    (instEndAlignedExpected ? 0 : 1);

static_assert(
    true == instEndAlignedExpected,
    "instruction end must be aligned, when using these addr_t and cell_t");
static_assert(
    2 == cellsPerInstExpected,
    "instruction end must be aligned, when using these addr_t and cell_t");



auto getTestBin() {
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
}



void testRAMController() {
  constexpr auto test_name = __FUNCTION__;

  RAMController controller;

#ifndef NDEBUG
  std::clog << "RAMController dump before loading binary:" << std::endl;
  controller.dump(std::clog);
#endif

  SHISA_CHECK_TEST(0 == controller.getProgramEnd(),
                   std::string{test_name} +
                       ": binary wasn't loaded to RAM but program end address "
                       "is not equal to zero");

  SHISA_CHECK_TEST(0 == controller.getBinEnd(),
                   std::string{test_name} +
                       ": binary wasn't loaded to RAM but binary end address "
                       "is not equal to zero");

  SHISA_CHECK_TEST(0 == controller.getBinDataAddr(),
                   std::string{test_name} +
                       ": binary wasn't loaded to RAM but binary data address "
                       "is not equal to zero");

  SHISA_CHECK_TEST(false == controller.isBinaryLoaded(),
                   std::string{test_name} +
                       ": binary wasn't loaded to RAM but marked as loaded");

  {
    Addr currAddr = 0;
    for (const auto cell : controller) {
      SHISA_CHECK_TEST(
          0 == cell, std::string{test_name} +
                         ": binary wasn't loaded to RAM but cell at address " +
                         std::to_string(currAddr) +
                         " not initialised with zero");
      currAddr++;
    }
  }

  Binary bin = getTestBin();
  controller.loadBin(bin);

#ifndef NDEBUG
  std::clog << "RAMController dump after loading binary:" << std::endl;
  controller.dump(std::clog);
#endif

  const Addr programEnd         = controller.getProgramEnd();
  const Addr expectedProgramEnd = bin.nInsts() * RAMController::cellsPerInst;
  SHISA_CHECK_TEST(expectedProgramEnd == programEnd,
                   std::string{test_name} + ": bad program end address: " +
                       std::to_string(programEnd) +
                       ", expected: " + std::to_string(expectedProgramEnd));

  const Addr binDataAddr         = controller.getBinDataAddr();
  const Addr expectedBinDataAddr = expectedProgramEnd;
  SHISA_CHECK_TEST(expectedBinDataAddr == binDataAddr,
                   std::string{test_name} + ": bad binary data address: " +
                       std::to_string(binDataAddr) +
                       ", expected: " + std::to_string(expectedBinDataAddr));

  const Addr   binEnd   = controller.getBinEnd();
  const size_t dataSize = sizeof(Binary::Data) / sizeof(Cell) +
                          (sizeof(Binary::Data) % sizeof(Cell) == 0 ? 0 : 1);
  const Addr expectedBinEnd = expectedProgramEnd + bin.nData() * dataSize;
  SHISA_CHECK_TEST(expectedBinEnd == binEnd,
                   std::string{test_name} +
                       ": bad binary end address: " + std::to_string(binEnd) +
                       ", expected: " + std::to_string(expectedBinEnd));

  SHISA_CHECK_TEST(true == controller.isBinaryLoaded(),
                   std::string{test_name} +
                       ": binary wasn't loaded to RAM but marked as loaded");

  {
    Addr currAddr = 0;
    for (const auto cell : controller) {
      if (currAddr < controller.getProgramEnd()) {
        const size_t instIdx = currAddr / RAMController::cellsPerInst;
        const Inst   inst    = bin.getISAModule().getRawInsts().at(instIdx);
        const size_t shiftSize =
            CHAR_BIT * (RAMController::cellsPerInst -
                        currAddr % RAMController::cellsPerInst - 1);
        const Cell expectedCellData =
            (inst >> shiftSize) & std::numeric_limits<Cell>::max();
        SHISA_CHECK_TEST(cell == expectedCellData,
                         std::string{test_name} + ": bad binary data " +
                             std::to_string(static_cast<unsigned>(cell)) +
                             " at address " + std::to_string(currAddr) +
                             ", expected " + std::to_string(expectedCellData));
      } else if (currAddr < controller.getBinEnd()) {
        const size_t dataIdx = (currAddr - controller.getProgramEnd()) /
                               RAMController::cellsPerData;
        const Binary::Data data = bin.getRawData().at(dataIdx);
        const size_t       shiftSize =
            CHAR_BIT * (RAMController::cellsPerData -
                        currAddr % RAMController::cellsPerData - 1);
        const Cell expectedCellData =
            (data >> shiftSize) & std::numeric_limits<Cell>::max();
        SHISA_CHECK_TEST(cell == expectedCellData,
                         std::string{test_name} + ": bad binary data " +
                             std::to_string(static_cast<unsigned>(cell)) +
                             " at address " + std::to_string(currAddr) +
                             ", expected " + std::to_string(expectedCellData));
      } else {
        break;
      }

      currAddr++;
    }
  }
}



void testLazyBased() {}



int main() {
  try {
    testRAMController();
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
