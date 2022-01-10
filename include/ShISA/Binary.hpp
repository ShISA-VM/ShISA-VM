#pragma once

#include <ShISA/ISAModule.hpp>
#include <ShISA/Inst.hpp>

#include <vector>



namespace shisa {

template <typename inst_t, typename data_t>
class BinaryBase {
public:
  using Data       = data_t;
  using Inst       = inst_t;

  using ISAModule  = ISAModuleBase<Inst>;
  using BinaryData = std::vector<Data>;

private:
  ISAModule  M;
  BinaryData binData;

public:
  BinaryBase(ISAModule &&otherModule, BinaryData &&otherBData)
      : M{otherModule}, binData{otherBData} {}

  [[nodiscard]] auto getISAModule() const -> const ISAModule & { return M; }

  [[nodiscard]] auto getRawData() const -> const BinaryData & {
    return binData;
  }

  [[nodiscard]] auto nInsts() const -> size_t { return M.nInsts(); }

  [[nodiscard]] auto nData() const -> size_t { return binData.size(); }
};

using Binary = BinaryBase<Inst, uint16_t>;

} // namespace shisa
