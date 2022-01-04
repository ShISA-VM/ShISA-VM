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

template <typename reg_t, typename addr_t, typename cell_t, size_t n_regs>
requires(std::unsigned_integral<Reg> &&std::unsigned_integral<Addr>
             &&std::unsigned_integral<Cell>) class SimBase {
public:
  using Reg  = reg_t;
  using Addr = addr_t;
  using Cell = cell_t;

  using CPU = CpuBase<Reg, Addr, Cell, n_regs>;

private:
  CPU cpu;

protected:
  auto getState() -> CPU & { return cpu; }

public:
  SimBase(const Binary &b) { cpu.loadBin(b); }

  virtual void execute() = 0;

  void dumpState(std::ostream &os) const { cpu.dump(os); }

  auto getState() const -> const CPU & { return cpu; }

  auto fetchNext() -> Inst::RawInst { return cpu.fetchNext(); }

  void processAdd(int dstReg, int srcLReg, int srcRReg) {
    Reg res = cpu.readReg(srcLReg) + cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processSub(int dstReg, int srcLReg, int srcRReg) {
    Reg res = cpu.readReg(srcLReg) - cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processMul(int dstReg, int srcLReg, int srcRReg) {
    Reg res = cpu.readReg(srcLReg) * cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processDiv(int dstReg, int srcLReg, int srcRReg) {
    Reg rReg = cpu.readReg(srcRReg);
    if (rReg == 0) {
      cpu.setPCToEnd();
      //TODO: maybe need to write about exception somewhere in MMIO
      return;
    }
    Reg res = cpu.readReg(srcLReg) / cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processAnd(int dstReg, int srcLReg, int srcRReg) {
    Reg res = cpu.readReg(srcLReg) & cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processOr(int dstReg, int srcLReg, int srcRReg) {
    Reg res = cpu.readReg(srcLReg) | cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  //TODO: isn't it sub?
  void processCmp(int dstReg, int srcLReg, int srcRReg) {
    Reg res = cpu.readReg(srcLReg) - cpu.readReg(srcRReg);
    cpu.writeReg(dstReg, res);
  }

  void processNot(int dstReg, int srcLReg, int /*srcRReg*/) {
    Reg res = ~(cpu.readReg(srcLReg));
    cpu.writeReg(dstReg, res);
  }

  void processJmpTrue(int /*dstReg*/, int srcLReg, int srcRReg) {
    Reg cond = cpu.readReg(srcLReg);
    if (cond == 0) {
      Reg jmpTo = cpu.readReg(srcRReg);
      cpu.setPC(jmpTo);
    }
  }

  void processLoad(int dstReg, int srcLReg, int /*srcRReg*/) {
    Reg addr = cpu.readReg(srcLReg);
    cpu.writeReg(dstReg, cpu.readWordFromRAM(addr));
  }

  void processStore(int /*dstReg*/, int srcLReg, int srcRReg) {
    Reg addr = cpu.readReg(srcLReg);
    Reg data = cpu.readReg(srcRReg);
    cpu.writeWordToRAM(addr, data);
  }

  void processPush(int /*dstReg*/, int srcLReg, int /*srcRReg*/) {
    cpu.storeRegOnStack(srcLReg);
  }

  void processPop(int dstReg, int /*srcLReg*/, int /*srcRReg*/) {
    cpu.loadRegFromStack(dstReg);
  }

  void processCall(int dstReg, int /*srcLReg*/, int /*srcRReg*/) {
    Reg jmpTo = cpu.readReg(dstReg);
    cpu.storePCOnStack();
    cpu.storeRegsOnStack();
    cpu.setPC(jmpTo);
  }

  void processRet(int /*dstReg*/, int /*srcLReg*/, int /*srcRReg*/) {
    cpu.loadRegsFromStack();
    cpu.loadPCFromStack();
  }
};

#define USING_SIM_BASE(sim_base_alias)                                         \
  using sim_base_alias::dumpState;                                             \
  using sim_base_alias::getState;                                              \
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
