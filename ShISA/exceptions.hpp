#pragma once

#include "Inst.hpp"

#include <array>
#include <exception>
#include <string>
#include <string_view>



namespace shisa {

class Exception : public std::exception {
private:
  std::string msg;

protected:
  auto appendMsg(std::string_view str) -> std::string & {
    return msg.append(str);
  }

public:
  explicit Exception() : msg{"shisa::Exception"} {}
  explicit Exception(std::string_view message) : msg{message.data()} {}

  [[nodiscard]] auto what() const noexcept -> const char * override {
    return msg.c_str();
  }
};



template <typename T>
concept IsInst = std::is_base_of<Inst, T>::value;

template <IsInst InstClass>
class UnexpectedInst : public Exception {
private:
  static constexpr size_t n = InstClass::opCodes.size();

  InstClass             inst;
  std::array<OpCode, n> expectedOps;

public:
  UnexpectedInst(InstClass *i, const std::array<OpCode, n> &opCodes)
      : Exception{"UnexpectedInst"}, inst{*i}, expectedOps{opCodes} {}

  [[nodiscard]] auto got() const noexcept -> InstClass { return inst; }

  [[nodiscard]] auto expected() const noexcept
      -> const std::array<OpCode, n> & {
    return expectedOps;
  }
};



template <typename T>
class InvalidValue : public Exception {
public:
  InvalidValue(const char *var_name, T var_val) : Exception{var_name} {
    appendMsg(" == ");
    appendMsg(std::to_string(var_val));
  }
};



class InvalidInst : public Exception {
public:
  InvalidInst(Inst inst) : Exception{"Invalid instruction: "} {
    appendMsg("with opcode");
    appendMsg(std::to_string(static_cast<unsigned>(inst.getOpCode())));
  }
};



class ProgramEnd : public Exception {
public:
  ProgramEnd() : Exception{"ProgramEnd"} {}
};



class StackOverflow : public Exception {
public:
  StackOverflow() : Exception{"StackOverflow"} {}
};



class StackUnderflow : public Exception {
public:
  StackUnderflow() : Exception{"StackUnderflow"} {}
};

} // namespace shisa

