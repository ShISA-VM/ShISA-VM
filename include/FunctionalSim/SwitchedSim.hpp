#pragma once

#include <FunctionalSim/Sim.hpp>
#include <ShISA/ISAModule.hpp>
#include <ShISA/Inst.hpp>
#include <exceptions.hpp>

#include <cstdint>



namespace shisa::fsim {

template <typename reg_t = uint16_t, typename addr_t = uint16_t,
          typename cell_t = uint8_t, size_t nRegs = NREGS>
class SwitchedSim final : public SimBase<reg_t, addr_t, cell_t, nRegs> {
public:
  using Reg  = reg_t;
  using Addr = addr_t;
  using Cell = cell_t;

  using Sim = SimBase<Reg, Addr, Cell, nRegs>;

  USING_SIM_BASE(Sim);

  SwitchedSim(const Binary &b) : Sim{b} {}

  void executeOne() override {
    const shisa::Inst inst = fetchNext();

    const auto [op, dst, srcL, srcR] = inst.decode();
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
      throw shisa::InvalidInst{inst};
      break;
    }
  }
};

template <class S>
concept isSwitchedSim =
    std::is_same_v<S, SwitchedSim<typename S::Reg, typename S::Addr,
                                  typename S::Cell, shisa::NREGS>>;

} // namespace shisa::fsim
