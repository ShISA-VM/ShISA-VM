#include "FunctionalSim.hpp"

#include <exceptions.hpp>
#include <ISAModule.hpp>
#include <Inst.hpp>

#include <cstdint>



namespace fsim {

template <typename addr_t = uint16_t, typename memCell_t = uint8_t,
          typename reg_t = uint16_t, size_t nRegs = shisa::NREGS,
          template <typename Addr, typename MemCell> class MemModel = LazyMem>
class SwitchedSim final
    : public SimBase<addr_t, memCell_t, reg_t, nRegs, MemModel> {
  using SimBase = SimBase<addr_t, memCell_t, reg_t, nRegs, MemModel>;

  using SimBase::fetchNext;
  using SimBase::processAdd;
  using SimBase::processAnd;
  using SimBase::processCmp;
  using SimBase::processDiv;
  using SimBase::processJmp;
  using SimBase::processJmpTrue;
  using SimBase::processLoad;
  using SimBase::processMul;
  using SimBase::processNot;
  using SimBase::processOr;
  using SimBase::processStore;
  using SimBase::processSub;

public:
  SwitchedSim(const shisa::ISAModule &m) : SimBase{m} {}

  void execute() override {
    const shisa::Inst inst     = fetchNext();
    auto [op, dst, srcL, srcR] = inst.decode();
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
    case shisa::OpCode::CMP:
      processCmp(dst, srcL, srcR);
      break;
    case shisa::OpCode::NOT:
      processNot(dst, srcL, srcR);
      break;
    case shisa::OpCode::JMP:
      processJmp(dst, srcL, srcR);
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
    default:
      throw shisa::InvalidInst{inst};
      break;
    }
  }
};

} // namespace fsim

