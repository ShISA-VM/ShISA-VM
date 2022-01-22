#pragma once

#include "Inst.hpp"

#include <cstdint>
#include <vector>



namespace shisa {

template <typename inst_t>
class ISAModuleBase {
  std::vector<inst_t> insts;

public:
  using RawInst = typename inst_t::RawInst;

  ISAModuleBase() = default;

  ISAModuleBase(const std::vector<inst_t> &i) : insts{i} {}

  ISAModuleBase(std::vector<inst_t> &&i) : insts{i} {}

  [[nodiscard]] auto getInsts() const -> const std::vector<inst_t> & {
    return insts;
  }

  [[nodiscard]] auto nInsts() const -> size_t { return insts.size(); }

  auto begin() -> typename std::vector<inst_t>::iterator {
    return insts.begin();
  }

  auto end() -> typename std::vector<inst_t>::iterator { return insts.end(); }

  [[nodiscard]] auto begin() const ->
      typename std::vector<inst_t>::const_iterator {
    return insts.begin();
  }

  [[nodiscard]] auto end() const ->
      typename std::vector<inst_t>::const_iterator {
    return insts.end();
  }
};

using ISAModule = ISAModuleBase<Inst>;

} // namespace shisa
