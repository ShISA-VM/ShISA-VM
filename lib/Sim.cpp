#include <FunctionalSim/SwitchedSim.hpp>
#include <ShISA/ISAModule.hpp>

int main() {
  shisa::fsim::SwitchedSim sim{shisa::Binary{shisa::ISAModule{}, {}}};
}
