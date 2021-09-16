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
  std::array<reg_t, nRegs> regs = {0, 1};

public:
  RegisterFile();

  void writeReg(int r, reg_t data) {
    if ((r != 0) && (r != 1)) {
      regs[r] = data;
    }
  }

  auto readReg(int r) -> reg_t { return regs[r].getValue(); }
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
  MemModel() = default;

  void loadBin(shisa::ISAModule *m) {
    addr_t currAddr = 0;
    for (const auto inst : m->getRawInsts()) {
      currAddr = loadInst(inst, currAddr);
    }

    programEnd   = currAddr;
    binaryLoaded = true;
  }

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

  auto read(addr_t addr) -> cell_t { return storage->get(addr); }
  void write(addr_t addr, cell_t data) { storage->set(addr, data); }
};



template <typename addr_t, typename cell_t, typename reg_t, size_t nRegs,
          template <typename Addr, typename Cell> class MemStorage>
class Cpu {
  using RF    = RegisterFile<reg_t, nRegs>;
  using RAM_t = RAM<addr_t, cell_t, MemStorage>;

  RF *   regFile;
  RAM_t *ram;
  addr_t PC = 0;

  constexpr static bool instEndAligned =
      (sizeof(shisa::ISAModule::RawInst) % sizeof(cell_t)) == 0;
  constexpr static size_t cellsPerInst =
      sizeof(shisa::ISAModule::RawInst) / sizeof(cell_t) +
      (instEndAligned ? 0 : 1);

  void PCIncrement() { PC += cellsPerInst; }

public:
  Cpu() {
    regFile = new RF;
    ram     = new RAM_t;
  }

  void loadBin(shisa::ISAModule *m) { ram->loadBin(m); }

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
};

} // namespace fsim

