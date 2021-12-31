#pragma once

#include "RegisterFile.hpp"
#include "RAMController.hpp"

#include <ShISA/Binary.hpp>
#include <ShISA/ISAModule.hpp>
#include <exceptions.hpp>

#include <array>
#include <concepts>
#include <iomanip>
#include <limits>
#include <ostream>



namespace shisa::fsim {

template <typename reg_t, typename addr_t, typename cell_t, size_t nRegs>
requires(std::unsigned_integral<reg_t> &&std::unsigned_integral<addr_t>
             &&std::unsigned_integral<cell_t>) class CpuBase {
public:
  using Reg  = reg_t;
  using Addr = addr_t;
  using Cell = cell_t;

  using RegisterFile  = RegisterFileBase<Reg, nRegs>;
  using RAMController = RAMControllerBase<Addr, cell_t>;

  constexpr static size_t NREGS = nRegs;

  constexpr static bool instEndAligned =
      (sizeof(ISAModule::RawInst) % sizeof(Cell)) == 0;
  constexpr static size_t cellsPerInst =
      sizeof(ISAModule::RawInst) / sizeof(Cell) + (instEndAligned ? 0 : 1);

  constexpr static bool   regEndAligned = (sizeof(Reg) % sizeof(Cell)) == 0;
  constexpr static size_t cellsPerReg =
      sizeof(Reg) / sizeof(Cell) + (instEndAligned ? 0 : 1);

private:
  RegisterFile  regFile;
  RAMController RAMControl;

  Addr PC = 0;
  Addr SP = 0;

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

  void loadBin(const Binary b) {
    RAMControl.loadBin(b);
    SP = RAMControl.getBinEnd();
  }

  void PCIncrement() {
    PC += cellsPerInst;
    if (PC >= RAMControl.getProgramEnd()) {
      throw ProgramEnd();
    }
  }

  void SPDecrement() {
    SP -= cellsPerReg;

    if (SP < RAMControl.getProgramEnd()) {
      throw StackUnderflow{};
    }
  }

  void SPIncrement() {
    SP += cellsPerReg;
    if (SP > (RAMControl.getProgramEnd() + STACK_OFFSET)) {
      throw StackOverflow{};
    }
  }

  auto fetchNext() -> Inst::RawInst {
    Inst::RawInst inst = 0;
    for (int i = 0; i < cellsPerInst; i++) {
      inst |= (RAMControl.read(PC + i)) << (cellsPerInst - i);
    }

    PCIncrement();
    return inst;
  }

  auto readReg(int r) const -> Reg { return regFile.read(r); }
  void writeReg(int r, Reg data) { regFile.write(r, data); }

  auto readFromRAM(Addr addr) const -> Cell { return RAMControl.read(addr); }
  void writeToRAM(Addr addr, Cell data) { RAMControl.write(addr, data); }

  auto readRegFromRAM(Addr addr) const -> Reg {
    Reg res = 0;
    for (int i = 0; i < cellsPerReg; i++) {
      res |= (RAMControl.read(addr + i)) << (cellsPerReg - i);
    }
    return res;
  }

  void writeRegToRAM(Addr addr, Reg data) {
    for (int i = 0; i < cellsPerReg; i++) {
      Cell cellData =
          (data >> (cellsPerReg - i)) && std::numeric_limits<Cell>::max();
      RAMControl.write(addr + i, cellData);
    }
  }

  void setPC(Addr addr) { PC = addr; }

  auto loadFromStack() -> Reg {
    Reg data = readRegFromRAM(SP);
    SPDecrement();
    return data;
  }

  void storeRegOnStack(Reg data) {
    writeRegToRAM(SP, data);
    SPIncrement();
  }

  void storePCOnStack() {
    writeRegToRAM(SP, PC + cellsPerInst);
    SPIncrement();
  }

  void loadRegsFromStack() {
    for (auto &reg : regFile) {
      reg = loadFromStack();
    }
  }

  void storeRegsOnStack() {
    for (const auto reg : regFile) {
      storeRegOnStack(reg);
    }
  }
};

} // namespace shisa::fsim
