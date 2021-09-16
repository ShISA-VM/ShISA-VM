#pragma once

#include "ShISAInst.hpp"

#include <cstdint>
#include <vector>



namespace shisa {

template <typename inst_t>
class ISAModuleBase {
  std::vector<inst_t> insts;

public:
  ISAModuleBase();

  using RawInst = typename inst_t::RawInst;

  auto getRawInsts() const -> const std::vector<inst_t> &;

  auto begin() -> typename std::vector<inst_t>::iterator;
  auto end() -> typename std::vector<inst_t>::iterator;

  [[nodiscard]] auto begin() const ->
      typename std::vector<inst_t>::const_iterator;
  [[nodiscard]] auto end() const ->
      typename std::vector<inst_t>::const_iterator;
};

using ISAModule = ISAModuleBase<Inst>;

} // namespace shisa

