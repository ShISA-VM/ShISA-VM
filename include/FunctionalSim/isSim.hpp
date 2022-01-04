#pragma once

#include "FunctionalSim/Sim.hpp"

#include <concepts>

namespace shisa::fsim {

template <class S>
concept isSim = std::derived_from<S, SimBase<typename S::Reg, typename S::Addr,
                                              typename S::Cell, shisa::NREGS>>;

} // namespace shisa::fsim
