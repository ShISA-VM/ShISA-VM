#pragma once

#include <FunctionalSim/Sim.hpp>
#include <ShISA/ISAModule.hpp>
#include <ShISA/Inst.hpp>
#include <exceptions.hpp>

#include <cstdint>
#include <functional>



namespace shisa::fsim {

template <typename reg_t = uint16_t, typename addr_t = uint16_t,
          typename cell_t = uint8_t, size_t nRegs = NREGS>
class SubroutinedSim final : public SimBase<reg_t, addr_t, cell_t, nRegs> {
public:
  using Reg  = reg_t;
  using Addr = addr_t;
  using Cell = cell_t;

  using Sim = SimBase<Reg, Addr, Cell, nRegs>;

  USING_SIM_BASE(Sim);

private:
  const std::array<const std::function<void(int, int, int)>,
                   Inst::opCodes.size()>
      routines = {
          [this](int dst, int srcL, int srcR) { processAdd(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processSub(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processMul(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processDiv(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processAnd(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processOr(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processXor(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processNot(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processCmp(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) {
            processJmpTrue(dst, srcL, srcR);
          },
          [this](int dst, int srcL, int srcR) { processLoad(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) {
            processStore(dst, srcL, srcR);
          },
          [this](int dst, int srcL, int srcR) { processPush(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processPop(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processCall(dst, srcL, srcR); },
          [this](int dst, int srcL, int srcR) { processRet(dst, srcL, srcR); },
      };

public:
  SubroutinedSim(const Binary &b) : Sim{b} {}

  void executeOne() override {
    const shisa::Inst inst = fetchNext();

    const auto [op, dst, srcL, srcR] = inst.decode();
    routines[static_cast<size_t>(op)](dst, srcL, srcR);
  }
};

template <class S>
concept isSubroutinedSim =
    std::is_same_v<S, SubroutinedSim<typename S::Reg, typename S::Addr,
                                     typename S::Cell, shisa::NREGS>>;

} // namespace shisa::fsim
