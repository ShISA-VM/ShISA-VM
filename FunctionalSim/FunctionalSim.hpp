#pragma once

#include <ISAModule.hpp>
#include <exceptions.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <map>



namespace fsim {

template <typename reg_t, size_t nRegs>
class RegisterFile {
  using RegArr = std::array<reg_t, nRegs>;
  RegArr regs  = {0, 1};

public:
  RegisterFile();

  void writeReg(int r, reg_t data) {
    if ((r != 0) && (r != 1)) {
      regs[r] = data;
    }
  }

  auto readReg(int r) -> reg_t { return regs[r].getValue(); }

  auto begin() -> typename RegArr::iterator { return regs.begin() + 2; }

  auto end() -> typename RegArr::iterator { return regs.end(); }

  auto begin() const -> typename RegArr::const_iterator {
    return regs.begin() + 2;
  }

  auto end() const -> typename RegArr::const_iterator { return regs.end(); }
};



template <typename addr_t, typename cell_t, typename Storage>
class MemModel {
public:
  constexpr static bool instEndAligned =
      (sizeof(shisa::ISAModule::RawInst) % sizeof(cell_t)) == 0;
  constexpr static size_t cellsPerInst =
      sizeof(shisa::ISAModule::RawInst) / sizeof(cell_t) +
      (instEndAligned ? 0 : 1);

  static_assert(std::numeric_limits<addr_t>::is_integer &&
                    !std::numeric_limits<addr_t>::is_signed,
                "addr_t must be an unsigned integer type");
  static_assert(std::numeric_limits<cell_t>::is_integer &&
                    !std::numeric_limits<cell_t>::is_signed,
                "cell_t must be an unsigned integer type");

private:
  Storage mem;

  using inst_t  = shisa::ISAModule::RawInst;
  using split_t = std::array<cell_t, cellsPerInst>;

  auto splitInst(inst_t inst) -> split_t {
    split_t splits;
    for (int i = 0; i < cellsPerInst; i++) {
      splits[i] = (inst >> i) & std::numeric_limits<cell_t>::max;
    }
    return std::move(splits);
  }

  auto loadInst(inst_t inst, addr_t addr) -> addr_t {
    std::array<cell_t, cellsPerInst> splits = splitInst(inst);
    for (const auto split : splits) {
      mem[addr++] = split;
    }

    return addr;
  }

  bool   binaryLoaded = false;
  addr_t programEnd   = 0;

public:
  void loadBin(shisa::ISAModule *m) {
    addr_t currAddr = 0;
    for (const auto inst : m->getRawInsts()) {
      currAddr = loadInst(inst, currAddr);
    }

    programEnd   = currAddr;
    binaryLoaded = true;
  }

  auto getProgramEnd() const -> addr_t { return programEnd; }

  auto get(addr_t addr) -> cell_t { return mem[addr]; }
  void set(addr_t addr, cell_t data) { mem[addr] = data; }
};



template <typename addr_t, typename cell_t>
using LazyMem = MemModel<addr_t, cell_t, std::map<addr_t, cell_t>>;



template <typename addr_t, typename cell_t>
using FullMem =
    MemModel<addr_t, cell_t,
             std::array<cell_t, std::numeric_limits<addr_t>::max + 1>>;



template <typename addr_t, typename cell_t,
          template <typename Addr, typename Cell> class MemModel>
class RAM {
  using MemStorage = MemModel<addr_t, cell_t>;
  MemStorage *storage;

public:
  RAM() { storage = new MemStorage; }

  void loadBin(shisa::ISAModule *m) { storage->loadBin(m); }

  auto getProgramEnd() const -> addr_t { return storage->getProgrammEnd; }

  auto read(addr_t addr) -> cell_t { return storage->get(addr); }
  void write(addr_t addr, cell_t data) { storage->set(addr, data); }
};



template <typename addr_t, typename cell_t, typename reg_t, size_t nRegs,
          template <typename Addr, typename Cell> class MemStorage>
class Cpu {
  using RF  = RegisterFile<reg_t, nRegs>;
  using RAM = RAM<addr_t, cell_t, MemStorage>;

  RF *   regFile;
  RAM *  ram;
  addr_t PC = 0;
  addr_t SP = 0;

  constexpr static bool instEndAligned =
      (sizeof(shisa::ISAModule::RawInst) % sizeof(cell_t)) == 0;
  constexpr static size_t cellsPerInst =
      sizeof(shisa::ISAModule::RawInst) / sizeof(cell_t) +
      (instEndAligned ? 0 : 1);

  constexpr static bool   regEndAligned = (sizeof(reg_t) % sizeof(cell_t)) == 0;
  constexpr static size_t cellsPerReg =
      sizeof(reg_t) / sizeof(cell_t) + (instEndAligned ? 0 : 1);

  void PCIncrement() {
    PC += cellsPerInst;
    if (PC >= ram->getProgramEnd()) {
      throw shisa::ProgramEnd();
    }
  }

  void SPDecrement() {
    SP -= cellsPerReg;

    if (SP < ram->getProgramEnd()) {
      throw shisa::StackUnderflow{};
    }
  }

  void SPIncrement() {
    SP += cellsPerReg;
    if (SP > (ram->getProgramEnd() + RAM::stackOffset)) {
      throw shisa::StackOverflow{};
    }
  }

public:
  Cpu() {
    regFile = new RF;
    ram     = new RAM;
  }

  ~Cpu() {
    delete regFile;
    delete ram;
  }

  void loadBin(shisa::ISAModule *m) {
    ram->loadBin(m);
    SP = ram->getProgramEnd();
  }

  auto fetchNext() -> shisa::Inst::RawInst {
    shisa::Inst::RawInst inst = 0;
    for (int i = 0; i < cellsPerInst; i++) {
      inst |= (ram->read(PC + i)) << (cellsPerInst - i);
    }

    PCIncrement();
    return inst;
  }

  auto readReg(int r) -> reg_t { return regFile->readReg(r); }
  void writeReg(int r, reg_t data) { regFile->writeReg(r, data); }

  auto readFromRAM(addr_t addr) -> cell_t { return ram->read(addr); }
  void writeToRAM(addr_t addr, cell_t data) { ram->write(addr, data); }

  auto readRegFromRAM(addr_t addr) -> reg_t {
    reg_t res = 0;
    for (int i = 0; i < cellsPerReg; i++) {
      res |= (ram->read(addr + i)) << (cellsPerReg - i);
    }
    return res;
  }

  void writeRegToRAM(addr_t addr, reg_t data) {
    for (int i = 0; i < cellsPerReg; i++) {
      cell_t cellData =
          (data >> (cellsPerReg - i)) && std::numeric_limits<cell_t>::max;
      ram->write(addr + i, cellData);
    }
  }

  auto loadFromStack() -> reg_t {
    reg_t data = readRegFromRAM(SP);
    SPDecrement();
    return data;
  }

  void storeOnStack(reg_t data) {
    writeRegToRAM(SP, data);
    SPIncrement();
  }

  void storePCOnStack() {
    writeRegToRAM(SP, PC + cellsPerInst);
    SPIncrement();
  }

  void LoadRegsFromStack() {
    for (auto &reg : *regFile) {
      reg = loadFromStack();
    }
  }

  void storeRegsOnStack() {
    for (const auto reg : *regFile) {
      storeOnStack(reg);
    }
  }
};



template <typename addr_t, typename memCell_t, typename reg_t, size_t nRegs,
          template <typename Addr, typename MemCell> class MemStorage>
class SimBase {
  shisa::ISAModule *ISAModule;

  using cpu_t = Cpu<addr_t, memCell_t, reg_t, nRegs, MemStorage>;
  cpu_t *state;

protected:
  auto getState() -> cpu_t * { return state; }

public:
  SimBase(const shisa::ISAModule &m) {
    ISAModule = new shisa::ISAModule(m);
    state     = new cpu_t();
    state->loadBin(ISAModule);
  }

  ~SimBase() {
    delete ISAModule;
    delete state;
  }

  virtual void execute() = 0;

  auto fetchNext() -> shisa::Inst::RawInst { return state->fetchNext(); }

  void processAdd(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = state->readReg(srcLReg) + state->readReg(srcRReg);
    state->writeReg(dstReg, res);
  }

  void processSub(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = state->readReg(srcLReg) - state->readReg(srcRReg);
    state->writeReg(dstReg, res);
  }

  void processMul(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = state->readReg(srcLReg) * state->readReg(srcRReg);
    state->writeReg(dstReg, res);
  }

  void processDiv(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = state->readReg(srcLReg) / state->readReg(srcRReg);
    state->writeReg(dstReg, res);
  }

  void processAnd(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = state->readReg(srcLReg) & state->readReg(srcRReg);
    state->writeReg(dstReg, res);
  }

  void processOr(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = state->readReg(srcLReg) | state->readReg(srcRReg);
    state->writeReg(dstReg, res);
  }

  void processCmp(int dstReg, int srcLReg, int srcRReg) {
    reg_t res = state->readReg(srcLReg) <=> state->readReg(srcRReg);
    state->writeReg(dstReg, res);
  }

  void processNot(int dstReg, int srcLReg, int /*srcRReg*/) {
    reg_t res = ~(state->readReg(srcLReg));
    state->writeReg(dstReg, res);
  }

  void processJmp(int /*dstReg*/, int srcLReg, int /*srcRReg*/) {
    reg_t jmpTo = state->readReg(srcLReg);
    state->setPC(jmpTo);
  }

  void processJmpTrue(int /*dstReg*/, int srcLReg, int srcRReg) {
    reg_t cond = state->readReg(srcLReg);
    if (cond == 0) {
      reg_t jmpTo = state->readReg(srcRReg);
      state->setPC(jmpTo);
    }
  }

  void processLoad(int dstReg, int srcLReg, int /*srcRReg*/) {
    reg_t addr = state->readReg(srcLReg);
    state->writeReg(dstReg, state->readFromRAM(addr));
  }

  void processStore(int /*dstReg*/, int srcLReg, int srcRReg) {
    reg_t addr = state->readReg(srcLReg);
    reg_t data = state->readReg(srcRReg);
    state->writeToRAM(addr, data);
  }

  void processPush(int /*dstReg*/, int srcLReg, int /*srcRReg*/) {
    state->storeOnStack(state->readReg(srcLReg));
  }

  void processPop(int /*dstReg*/, int srcLReg, int /*srcRReg*/) {
    state->storePCOnStack();
    state->storeRegsOnStack();
    reg_t jmpTo = state->readReg(srcLReg);
    state->setPC(jmpTo);
  }

  void processCall(int dstReg, int /*srcLReg*/, int /*srcRReg*/) {
    state->writeReg(dstReg, state->loadFromStack());
  }

  void processRet(int /*dstReg*/, int srcLReg, int /*srcRReg*/) {
    state->loadRegsFromStack();
    reg_t jmpTo = state->loadFromStack();
    state->storeOnStack(state->readReg(srcLReg));
  }
};

#define USING_SIM_BASE(sim_base_alias)                                         \
  using sim_base_alias::fetchNext;                                             \
  using sim_base_alias::processAdd;                                            \
  using sim_base_alias::processAnd;                                            \
  using sim_base_alias::processCmp;                                            \
  using sim_base_alias::processDiv;                                            \
  using sim_base_alias::processJmp;                                            \
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

} // namespace fsim

