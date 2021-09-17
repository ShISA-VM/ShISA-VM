#pragma once

#include <array>
#include <cstdint>
#include <tuple>



namespace shisa {

enum class OpCode {
  ADD  = 0x00,
  SUB  = 0x01,
  MUL  = 0x02,
  DIV  = 0x03,
  AND  = 0x04,
  OR   = 0x05,
  CMP  = 0x06,
  NOT  = 0x07,
  JMP  = 0x08,
  JTR  = 0x09,
  LD   = 0x0a,
  ST   = 0x0b,
  PUSH = 0x0c,
  POP  = 0x0d,
  CALL = 0x0e,
  RET  = 0x0f
};

constexpr std::size_t NREGS = 16;

template <typename Inst>
class InstBase {
  Inst inst;

public:
  InstBase(Inst i) : inst{i} {}

  operator Inst() const { return inst; }

  using RawInst = Inst;

  auto getOpCode() const;
  auto decode() const;
};

template <>
class InstBase<uint16_t> {
  uint16_t inst;

  static constexpr unsigned mask = 0x0f;

  static constexpr std::size_t srcRPos   = 0;
  static constexpr std::size_t srcLPos   = 1;
  static constexpr std::size_t dstPos    = 2;
  static constexpr std::size_t opCodePos = 3;

public:
  using RawInst = uint16_t;

  InstBase(RawInst i) : inst{i} {}

  [[nodiscard]] inline auto getOpCode() const -> OpCode {
    return static_cast<OpCode>((inst >> opCodePos) & mask);
  }

  [[nodiscard]] auto decode() const {
    int srcR = (inst >> srcRPos) & mask;
    int srcL = (inst >> srcLPos) & mask;
    int dst  = (inst >> dstPos) & mask;

    auto op = getOpCode();

    return std::make_tuple(op, dst, srcR, srcL);
  }

  constexpr static std::array opCodes = {
      OpCode::ADD,  OpCode::SUB, OpCode::MUL,  OpCode::DIV,
      OpCode::AND,  OpCode::OR,  OpCode::CMP,  OpCode::NOT,
      OpCode::JMP,  OpCode::JTR, OpCode::LD,   OpCode::ST,
      OpCode::PUSH, OpCode::POP, OpCode::CALL, OpCode::RET};
};

using Inst = InstBase<uint16_t>;

} // namespace shisa
