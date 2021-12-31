#pragma once

#include "CPU.hpp"

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
