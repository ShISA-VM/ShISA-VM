#pragma once

#include "Sim.hpp"

#include "PredecodedSim.hpp"
#include "PredecodedSubroutinedSim.hpp"
#include "SubroutinedSim.hpp"
#include "SwitchedSim.hpp"

#include <concepts>

namespace shisa::fsim {

template <class S>
concept isSim = std::derived_from<S, SimBase<typename S::Reg, typename S::Addr,
                                             typename S::Cell, shisa::NREGS>>;

template <isSim Sim>
static auto getSimName() -> std::string {
  if (shisa::fsim::isSwitchedSim<Sim>) {
    return std::move(std::string{"SwitchedSim"});
  }
  if (shisa::fsim::isPredecodedSim<Sim>) {
    return std::move(std::string{"PredecodedSim"});
  }
  if (shisa::fsim::isSubroutinedSim<Sim>) {
    return std::move(std::string{"SubroutinedSim"});
  }
  if (shisa::fsim::isPredecodedSubroutinedSim<Sim>) {
    return std::move(std::string{"PredecodedSubroutinedSim"});
  }
  return std::move(std::string{"UnknownSim"});
}

} // namespace shisa::fsim
