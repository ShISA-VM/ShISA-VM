#pragma once

#include <ShISA/Binary.hpp>
#include <ShISA/ISAModule.hpp>
#include <exceptions.hpp>

#include <array>
#include <climits>
#include <concepts>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <map>
#include <ostream>
#include <type_traits>
#include <utility>



namespace shisa::fsim {

template <typename reg_t, size_t nRegs>
requires(std::unsigned_integral<reg_t>) class RegisterFileBase {
public:
  using Reg = reg_t;

private:
  using RegArr = std::array<reg_t, nRegs>;
  RegArr regs  = {0, 1};

public:
  constexpr auto read(int r) const -> reg_t { return regs[r]; }

  constexpr void write(int r, reg_t data) {
    if ((r != 0) && (r != 1)) {
      regs[r] = data;
    }
  }

  void dump(std::ostream &os) const {
    os << "Register File dump\n";
    int regNumber = 0;
    for (const auto reg : regs) {
      os << "r" << std::setw(2) << std::left << std::dec << std::setfill(' ')
         << regNumber++ << " = 0x" << std::setw(2 * sizeof(reg_t)) << std::right
         << std::hex << std::setfill('0') << static_cast<unsigned>(reg) << "\n";
    }
  }

  constexpr auto begin() -> typename RegArr::iterator {
    return regs.begin() + FIRST_WRITABLE_REG;
  }

  constexpr auto end() -> typename RegArr::iterator { return regs.end(); }

  constexpr auto begin() const -> typename RegArr::const_iterator {
    return regs.begin() + FIRST_WRITABLE_REG;
  }

  constexpr auto end() const -> typename RegArr::const_iterator {
    return regs.end();
  }
};

using RegisterFile = RegisterFileBase<shisa::Reg, shisa::NREGS>;



template <typename addr_t, typename cell_t>
requires(std::unsigned_integral<addr_t>
             &&std::unsigned_integral<cell_t>) class RAMBase {
public:
  using Addr       = addr_t;
  using Cell       = cell_t;
  using MemStorage = std::array<Cell, std::numeric_limits<Addr>::max() + 1>;

  static_assert(std::numeric_limits<Addr>::is_integer &&
                    !std::numeric_limits<Addr>::is_signed,
                "addr_t must be an unsigned integer type");
  static_assert(std::numeric_limits<Cell>::is_integer &&
                    !std::numeric_limits<Cell>::is_signed,
                "cell_t must be an unsigned integer type");

private:
  MemStorage storage{};

public:
  void dump(std::ostream &os) const {
    os << "RAM dump\n";
    Addr addr = 0;
    for (const auto cell : storage) {
      os << "0x" << std::setw(2 * sizeof(Addr)) << std::right << std::hex
         << std::setfill('0') << addr++ << " = 0x"
         << std::setw(2 * sizeof(Cell)) << std::right << std::hex
         << std::setfill('0') << static_cast<unsigned>(cell) << "\n";
    }
  }

  auto begin() { return storage.begin(); }
  auto end() { return storage.end(); }

  auto begin() const { return storage.begin(); }
  auto end() const { return storage.end(); }

  auto read(Addr addr) const -> Cell { return storage[addr]; }
  void write(Addr addr, Cell data) { storage[addr] = data; }
};



template <typename addr_t, typename cell_t>
requires(std::unsigned_integral<addr_t>
             &&std::unsigned_integral<cell_t>) class RAMControllerBase {
public:
  using Addr = addr_t;
  using Cell = cell_t;

  static constexpr bool instEndAligned =
      (sizeof(ISAModule::RawInst) % sizeof(Cell)) == 0;
  static constexpr size_t cellsPerInst =
      sizeof(ISAModule::RawInst) / sizeof(Cell) + (instEndAligned ? 0 : 1);

  static constexpr bool dataEndAligned =
      (sizeof(Binary::Data) % sizeof(Cell)) == 0;
  static constexpr size_t cellsPerData =
      sizeof(Binary::Data) / sizeof(Cell) + (dataEndAligned ? 0 : 1);

private:
  using RAM = RAMBase<addr_t, cell_t>;
  RAM ram;

  bool binaryLoaded = false;

  addr_t binaryData = 0x0000;
  addr_t binaryEnd  = 0x0000;

public:
  using MemStorage = typename RAM::MemStorage;

  auto begin() { return ram.begin(); }
  auto end() { return ram.end(); }

  auto begin() const { return ram.begin(); }
  auto end() const { return ram.end(); }

  void dump(std::ostream &os) const {
    os << "RAMController dump\n";
    ram.dump(os);
  }

  void loadBin(const Binary &b) {
    addr_t currAddr = 0;

    for (const auto inst : b.getISAModule().getRawInsts()) {
      for (int i = cellsPerInst - 1; i >= 0; i--) {
        ram.write(currAddr++, (inst >> (i * sizeof(Cell) * CHAR_BIT)) &
                                  std::numeric_limits<cell_t>::max());
      }
    }

    binaryData = currAddr;

    for (const auto data : b.getRawData()) {
      for (int i = cellsPerData - 1; i >= 0; i--) {
        ram.write(currAddr++, (data >> (i * sizeof(Cell) * CHAR_BIT)) &
                                  std::numeric_limits<cell_t>::max());
      }
    }

    binaryEnd    = currAddr;
    binaryLoaded = true;
  }

  [[nodiscard]] auto getProgramEnd() const -> Addr { return binaryData; }

  [[nodiscard]] auto getBinEnd() const -> Addr { return binaryEnd; }

  [[nodiscard]] auto getBinDataAddr() const -> Addr { return binaryData; }

  [[nodiscard]] auto isBinaryLoaded() const -> bool { return binaryLoaded; }

  auto read(addr_t addr) const -> cell_t { return ram.read(addr); }

  void write(addr_t addr, cell_t data) { ram.write(addr, data); }
};



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



template <typename reg_t, typename addr_t, typename cell_t, size_t nRegs>
requires(std::unsigned_integral<reg_t> &&std::unsigned_integral<addr_t>
             &&std::unsigned_integral<cell_t>) class SimBase {
public:
  using Reg  = reg_t;
  using Addr = addr_t;
  using Cell = cell_t;

  using CPU = CpuBase<addr_t, cell_t, reg_t, nRegs>;

private:
  CPU cpu;

protected:
  auto getState() -> CPU { return cpu; }

public:
  SimBase(const Binary &b) { cpu.loadBin(b); }

  virtual void execute() = 0;

  void dump_state() const { cpu.dump(); }

  auto fetchNext() -> Inst::RawInst { return cpu.fetchNext(); }

  void processAdd(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = cpu.readReg(srcLReg) + cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processSub(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = cpu.readReg(srcLReg) - cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processMul(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = cpu.readReg(srcLReg) * cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processDiv(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = cpu.readReg(srcLReg) / cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processAnd(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = cpu.readReg(srcLReg) & cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processOr(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = cpu.readReg(srcLReg) | cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processCmp(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = cpu.readReg(srcLReg) - cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processNot(int dstReg, int srcLReg, int /*srcRReg*/) {
    reg_t res = ~(cpu.readReg(srcLReg));
    cpu.writeReg(dstReg, res);
  }

  void processJmpTrue(int /*dstReg*/, int srcLReg, int srcRReg) {
    reg_t cond = cpu.readReg(srcLReg);
    if (cond == 0) {
      reg_t jmpTo = cpu.readReg(srcRReg);
      cpu.setPC(jmpTo);
    }
  }

  void processLoad(int dstReg, int srcLReg, int /*srcRReg*/) {
    reg_t addr = cpu.readReg(srcLReg);
    cpu.writeReg(dstReg, cpu.readFromRAM(addr));
  }

  void processStore(int /*dstReg*/, int srcLReg, int srcRReg) {
    reg_t addr = cpu.readReg(srcLReg);
    reg_t data = cpu.readReg(srcRReg);
    cpu.writeToRAM(addr, data);
  }

  void processPush(int /*dstReg*/, int srcLReg, int /*srcRReg*/) {
    cpu.storeRegOnStack(cpu.readReg(srcLReg));
  }

  void processPop(int /*dstReg*/, int srcLReg, int /*srcRReg*/) {
    cpu.storePCOnStack();
    cpu.storeRegsOnStack();
    reg_t jmpTo = cpu.readReg(srcLReg);
    cpu.setPC(jmpTo);
  }

  void processCall(int dstReg, int /*srcLReg*/, int /*srcRReg*/) {
    cpu.writeReg(dstReg, cpu.loadFromStack());
  }

  void processRet(int /*dstReg*/, int srcLReg, int /*srcRReg*/) {
    cpu.loadRegsFromStack();
    reg_t jmpTo = cpu.loadFromStack();
    cpu.storeRegOnStack(cpu.readReg(srcLReg));
  }
};

#define USING_SIM_BASE(sim_base_alias)                                         \
  using sim_base_alias::dump_state;                                            \
  using sim_base_alias::fetchNext;                                             \
  using sim_base_alias::processAdd;                                            \
  using sim_base_alias::processAnd;                                            \
  using sim_base_alias::processCmp;                                            \
  using sim_base_alias::processDiv;                                            \
  using sim_base_alias::processJmpTrue;                                        \
  using sim_base_alias::processLoad;                                           \
  using sim_base_alias::processMul;                                            \
  using sim_base_alias::processNot;                                            \
  using sim_base_alias::processOr;                                             \
  using sim_base_alias::processStore;                                          \
  using sim_base_alias::processSub;                                            \
  using sim_base_alias::processPush;                                           \
  using sim_base_alias::processPop;                                            \
  using sim_base_alias::processCall;                                           \
  using sim_base_alias::processRet

} // namespace shisa::fsim
