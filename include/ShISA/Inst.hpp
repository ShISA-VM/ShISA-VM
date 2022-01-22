#pragma once

#include <array>
#include <climits>
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
  XOR  = 0x06,
  NOT  = 0x07,
  CMP  = 0x08,
  JTR  = 0x09,
  LD   = 0x0a,
  ST   = 0x0b,
  PUSH = 0x0c,
  POP  = 0x0d,
  CALL = 0x0e,
  RET  = 0x0f
};

constexpr std::size_t NREGS              = 16;
constexpr std::size_t FIRST_WRITABLE_REG = 2;

constexpr std::size_t STACK_OFFSET = 0x1000;

template <typename Inst>
class InstBase {
  Inst inst;

public:
  InstBase() : inst{} {}

  InstBase(Inst i) : inst{i} {}

  using RawInst = Inst;

  operator RawInst() const { return inst; }

  auto getOpCode() const;
  auto decode() const;
};

template <>
class InstBase<uint16_t> {
  uint16_t inst;

public:
  struct DecodedInst;

  static constexpr unsigned mask = 0x0f;

  static constexpr std::size_t srcRPos   = 0;
  static constexpr std::size_t srcLPos   = 1;
  static constexpr std::size_t dstPos    = 2;
  static constexpr std::size_t opCodePos = 3;

  static constexpr std::size_t nPos    = 4;
  static constexpr std::size_t posSize = sizeof(inst) * CHAR_BIT / nPos;

  static constexpr std::size_t srcRShift   = srcRPos * posSize;
  static constexpr std::size_t srcLShift   = srcLPos * posSize;
  static constexpr std::size_t dstShift    = dstPos * posSize;
  static constexpr std::size_t opCodeShift = opCodePos * posSize;

  using RawInst = uint16_t;

  InstBase() : inst{} {}

  InstBase(RawInst i) : inst{i} {}

  operator RawInst() const { return inst; }

  [[nodiscard]] auto decode() const -> DecodedInst { return DecodedInst{inst}; }

  constexpr static auto encode(OpCode opCode, int dst, int srcL, int srcR)
      -> RawInst {
    RawInst inst = 0;
    inst |= (static_cast<int>(opCode) & mask) << opCodeShift;
    inst |= (dst & mask) << dstShift;
    inst |= (srcL & mask) << srcLShift;
    inst |= (srcR & mask) << srcRShift;
    return inst;
  }

  constexpr static std::array opCodes = {
      OpCode::ADD,  OpCode::SUB, OpCode::MUL,  OpCode::DIV,
      OpCode::AND,  OpCode::OR,  OpCode::XOR,  OpCode::NOT,
      OpCode::CMP,  OpCode::JTR, OpCode::LD,   OpCode::ST,
      OpCode::PUSH, OpCode::POP, OpCode::CALL, OpCode::RET};

  struct DecodedInst {
    OpCode opCode;
    int    dst;
    int    srcL;
    int    srcR;

    DecodedInst(RawInst i) {
      opCode = static_cast<OpCode>((i >> opCodeShift) & mask);
      dst    = (i >> dstShift) & mask;
      srcL   = (i >> srcLShift) & mask;
      srcR   = (i >> srcRShift) & mask;
    }

    DecodedInst(OpCode o, int d, int l, int r)
        : opCode{o}, dst{d}, srcL{l}, srcR{r} {}
  };
};

using Reg  = uint16_t;
using Addr = uint16_t;
using Cell = uint8_t;
using Inst = InstBase<uint16_t>;

} // namespace shisa
