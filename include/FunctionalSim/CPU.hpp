#pragma once

#include "RAMController.hpp"
#include "RegisterFile.hpp"

#include <ShISA/Binary.hpp>
#include <ShISA/ISAModule.hpp>
#include <exceptions.hpp>

#include <array>
#include <concepts>
#include <iomanip>
#include <limits>
#include <ostream>
#include <ranges>



namespace shisa::fsim {

template <typename reg_t, typename addr_t, typename cell_t, size_t nRegs>
requires(std::unsigned_integral<reg_t> &&std::unsigned_integral<addr_t>
             &&std::unsigned_integral<cell_t>) class CpuBase {
public:
  using Reg  = reg_t;
  using Addr = addr_t;
  using Cell = cell_t;

  using RegisterFile  = RegisterFileBase<Reg, nRegs>;
  using RAMController = RAMControllerBase<Addr, Cell>;

  constexpr static size_t NREGS = nRegs;

  constexpr static bool instEndAligned =
      (sizeof(ISAModule::RawInst) % sizeof(Cell)) == 0;
  constexpr static size_t cellsPerInst =
      sizeof(ISAModule::RawInst) / sizeof(Cell) + (instEndAligned ? 0 : 1);

  constexpr static bool   regEndAligned = (sizeof(Reg) % sizeof(Cell)) == 0;
  constexpr static size_t cellsPerReg =
      sizeof(Reg) / sizeof(Cell) + (instEndAligned ? 0 : 1);

private:
  RegisterFile  regFile{};
  RAMController RAMControl{};

  Addr PC = 0;
  Addr SP = 0;

  bool reachEnd = true;

public:
  [[nodiscard]] auto getPC() const -> Addr { return PC; }
  [[nodiscard]] auto getSP() const -> Addr { return SP; }

  void dumpPC(std::ostream &os) const { os << "PC = " << PC << "\n"; }
  void dumpSP(std::ostream &os) const { os << "SP = " << SP << "\n"; }

  void dump(std::ostream &os) const {
    os << "Cpu state dump\n";
    dumpPC(os);
    dumpSP(os);
    regFile.dump(os);
    RAMControl.dump(os);
  }

  auto reg_begin() -> decltype(regFile.begin()) { return regFile.begin(); }
  auto reg_end() -> decltype(regFile.end()) { return regFile.end(); }

  auto reg_begin() const -> decltype(regFile.begin()) {
    return regFile.begin();
  }
  auto reg_end() const -> decltype(regFile.end()) { return regFile.end(); }

  auto regs_range() { return std::views::all(regFile); }

  auto regs_range() const { return std::views::all(regFile); }

  auto ram_begin() -> decltype(RAMControl.begin()) {
    return RAMControl.begin();
  }
  auto ram_end() -> decltype(RAMControl.end()) { return RAMControl.end(); }

  auto ram_begin() const -> decltype(RAMControl.begin()) {
    return RAMControl.begin();
  }
  auto ram_end() const -> decltype(RAMControl.end()) {
    return RAMControl.end();
  }

  auto ram_range() { return std::views::all(RAMControl); }

  auto ram_range() const { return std::views::all(RAMControl); }

  void loadBin(const Binary b) {
    RAMControl.loadBin(b);
    PC       = RAMControl.getProgramStart();
    SP       = RAMControl.getBinEnd();
    reachEnd = static_cast<bool>(PC >= RAMControl.getProgramEnd());
  }

  void PCIncrement() {
    PC += cellsPerInst;
    if (PC < RAMControl.getProgramEnd()) {
      reachEnd = false;
    } else if (PC == RAMControl.getProgramEnd()) {
      reachEnd = true;
    } else {
      PC -= cellsPerInst;
      throw ProgramEnd{};
    }
  }

  void SPRegIncrementBy(Addr nInc) {
    SP += nInc;
    if (SP > (RAMControl.getBinEnd() + STACK_OFFSET)) {
      SP -= nInc;
      throw StackOverflow{};
    }
  }

  void SPDecrementBy(Addr nDec) {
    if (SP < nDec) {
      throw StackUnderflow{};
    }

    SP -= nDec;

    if (SP < RAMControl.getBinEnd()) {
      SP += nDec;
      throw StackUnderflow{};
    }
  }

  void SPIncrementBy(Addr nInc) {
    SP += nInc;
    if (SP > (RAMControl.getBinEnd() + STACK_OFFSET)) {
      SP -= nInc;
      throw StackOverflow{};
    }
  }

  void SPDecrement() { SPDecrementBy(1); }

  void SPIncrement() { SPIncrementBy(1); }

  void SPRegDecrement() { SPDecrementBy(cellsPerReg); }

  void SPRegIncrement() { SPIncrementBy(cellsPerReg); }

  auto fetchNext() -> Inst::RawInst {
    if (reachEnd) {
      throw ProgramEnd{};
    }

    Inst::RawInst inst = readWordFromRAM(PC);
    PCIncrement();
    return inst;
  }

  auto readReg(int r) const -> Reg { return regFile.read(r); }
  void writeReg(int r, Reg data) { regFile.write(r, data); }

  auto readFromRAM(Addr addr) const -> Cell { return RAMControl.read(addr); }
  void writeToRAM(Addr addr, Cell data) { RAMControl.write(addr, data); }

  auto readWordFromRAM(Addr addr) const -> Reg {
    Reg res = 0;
    for (int i = 0; i < cellsPerReg; i++) {
      res |= static_cast<Reg>(RAMControl.read(addr + i))
             << (cellsPerReg - i - 1) * sizeof(Cell) * CHAR_BIT;
    }
    return res;
  }

  void writeWordToRAM(Addr addr, Reg data) {
    for (int i = 0; i < cellsPerReg; i++) {
      size_t shift    = cellsPerReg - i - 1;
      Cell   cellData = data >> (shift * sizeof(Cell) * CHAR_BIT);
      RAMControl.write(addr + i, cellData);
    }
  }

  void readRegFromRAM(Addr addr, int r) {
    Reg data = readWordFromRAM(addr);
    regFile.write(r, data);
  }

  void writeRegToRAM(Addr addr, int r) {
    Reg data = regFile.read(r);
    writeWordToRAM(addr, data);
  }

  void setPC(Addr addr) {
    PC = addr;
    if (PC < RAMControl.getProgramEnd()) {
      reachEnd = false;
    } else if (PC == RAMControl.getProgramEnd()) {
      reachEnd = true;
    } else {
      throw ProgramEnd{};
    }

    if (PC < RAMControl.getProgramStart()) {
      throw BadPC{};
    }
  }

  void setPCToEnd() {
    PC       = RAMControl.getProgramEnd();
    reachEnd = true;
  }

  auto loadFromStack() -> Cell {
    SPDecrement();
    return readFromRAM(SP);
  }

  void storeOnStack(Cell data) {
    writeToRAM(SP, data);
    SPIncrement();
  }

  void loadRegFromStack(int r) {
    SPRegDecrement();
    readRegFromRAM(SP, r);
  }

  void storeRegOnStack(int r) {
    writeRegToRAM(SP, r);
    SPRegIncrement();
  }

  void loadPCFromStack() {
    SPRegDecrement();
    setPC(readWordFromRAM(SP));
    //PCIncrement();
  }

  void storePCOnStack() {
    writeWordToRAM(SP, PC);
    SPRegIncrement();
  }

  void loadRegsFromStack() {
    for (int r :
         std::views::iota(FIRST_WRITABLE_REG, NREGS) | std::views::reverse) {
      loadRegFromStack(r);
    }
  }

  void storeRegsOnStack() {
    for (int r : std::views::iota(FIRST_WRITABLE_REG, NREGS)) {
      storeRegOnStack(r);
    }
  }
};

} // namespace shisa::fsim
