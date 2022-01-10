#pragma once

#include <ShISA/ISAModule.hpp>

#include <array>
#include <concepts>
#include <iomanip>
#include <ostream>



namespace shisa::fsim {

template <typename reg_t, size_t nRegs>
requires(std::unsigned_integral<reg_t>) class RegisterFileBase {
public:
  using Reg = reg_t;

private:
  using RegArr = std::array<reg_t, nRegs>;
  RegArr regs  = {0, 1};

public:
  constexpr auto read(int r) const -> reg_t { return regs[r]; }

  constexpr void write(int r, reg_t data) {
    if ((r != 0) && (r != 1)) {
      regs[r] = data;
    }
  }

  void dump(std::ostream &os) const {
    os << "Register File dump\n";
    int regNumber = 0;
    for (const auto reg : regs) {
      os << "r" << std::setw(2) << std::left << std::dec << std::setfill(' ')
         << regNumber++ << " = 0x" << std::setw(2 * sizeof(reg_t)) << std::right
         << std::hex << std::setfill('0') << static_cast<unsigned>(reg) << "\n";
    }
  }

  constexpr auto begin() -> typename RegArr::iterator {
    return regs.begin() + FIRST_WRITABLE_REG;
  }

  constexpr auto end() -> typename RegArr::iterator { return regs.end(); }

  constexpr auto begin() const -> typename RegArr::const_iterator {
    return regs.begin() + FIRST_WRITABLE_REG;
  }

  constexpr auto end() const -> typename RegArr::const_iterator {
    return regs.end();
  }

  constexpr auto rbegin() -> typename RegArr::iterator {
    return regs.rbegin();
  }

  constexpr auto rend() -> typename RegArr::iterator { return regs.rend() + FIRST_WRITABLE_REG; }

  constexpr auto rbegin() const -> typename RegArr::const_iterator {
    return regs.rbegin();
  }

  constexpr auto rend() const -> typename RegArr::const_iterator {
    return regs.rend() + FIRST_WRITABLE_REG;
  }
};

using RegisterFile = RegisterFileBase<shisa::Reg, shisa::NREGS>;

} // namespace shisa::fsim
