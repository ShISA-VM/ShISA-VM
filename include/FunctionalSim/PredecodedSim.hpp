#pragma once

#include <FunctionalSim/Sim.hpp>
#include <ShISA/ISAModule.hpp>
#include <ShISA/Inst.hpp>
#include <exceptions.hpp>

#include <algorithm>
#include <cstdint>



namespace shisa::fsim {

template <typename reg_t = uint16_t, typename addr_t = uint16_t,
          typename cell_t = uint8_t, size_t nRegs = NREGS>
class PredecodedSim final : public SimBase<reg_t, addr_t, cell_t, nRegs> {
  std::vector<shisa::Inst::DecodedInst> predecodedInsts{};

public:
  using Reg  = reg_t;
  using Addr = addr_t;
  using Cell = cell_t;

  using Sim = SimBase<Reg, Addr, Cell, nRegs>;

  USING_SIM_BASE(Sim);

  PredecodedSim(const Binary &b) : Sim{b} {
    const auto &insts = b.getISAModule();
    std::for_each(insts.begin(), insts.end(), [this](const shisa::Inst i) {
      predecodedInsts.push_back(i.decode());
    });
  }

  void executeOne() override {
    const auto & state = getState();
    const size_t instIdx =
        (state.getPC() - state.getProgramStart()) / Sim::CPU::cellsPerInst;
    PCIncrement();

    auto [op, dst, srcL, srcR] = predecodedInsts[instIdx];
    switch (op) {
    case shisa::OpCode::ADD:
      processAdd(dst, srcL, srcR);
      break;
    case shisa::OpCode::SUB:
      processSub(dst, srcL, srcR);
      break;
    case shisa::OpCode::MUL:
      processMul(dst, srcL, srcR);
      break;
    case shisa::OpCode::DIV:
      processDiv(dst, srcL, srcR);
      break;
    case shisa::OpCode::AND:
      processAnd(dst, srcL, srcR);
      break;
    case shisa::OpCode::OR:
      processOr(dst, srcL, srcR);
      break;
    case shisa::OpCode::XOR:
      processXor(dst, srcL, srcR);
      break;
    case shisa::OpCode::CMP:
      processCmp(dst, srcL, srcR);
      break;
    case shisa::OpCode::NOT:
      processNot(dst, srcL, srcR);
      break;
    case shisa::OpCode::JTR:
      processJmpTrue(dst, srcL, srcR);
      break;
    case shisa::OpCode::LD:
      processLoad(dst, srcL, srcR);
      break;
    case shisa::OpCode::ST:
      processStore(dst, srcL, srcR);
      break;
    case shisa::OpCode::PUSH:
      processPush(dst, srcL, srcR);
      break;
    case shisa::OpCode::CALL:
      processCall(dst, srcL, srcR);
      break;
    case shisa::OpCode::POP:
      processPop(dst, srcL, srcR);
      break;
    case shisa::OpCode::RET:
      processRet(dst, srcL, srcR);
      break;
    default:
      throw shisa::InvalidInst{Inst::encode(op, dst, srcL, srcR)};
      break;
    }
  }
};

template <class S>
concept isPredecodedSim =
    std::is_same_v<S, PredecodedSim<typename S::Reg, typename S::Addr,
                                    typename S::Cell, shisa::NREGS>>;

} // namespace shisa::fsim
