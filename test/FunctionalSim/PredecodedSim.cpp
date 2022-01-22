#include "SimTester.hpp"

#include <FunctionalSim/PredecodedSim.hpp>
#include <exceptions.hpp>

using Reg  = uint16_t;
using Addr = uint16_t;
using Cell = uint8_t;

using SimTester = shisa::test::SimTester<Reg, Addr, Cell, shisa::NREGS,
                                         shisa::fsim::PredecodedSim>;



int main() {
  try {
    SimTester::runTests();
  } catch (const shisa::test::Exception &e) {
    std::cerr << __FILE__ ": test fail: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (const std::exception &e) {
    std::cerr << __FILE__ ": test fail: exception caught in main: " << e.what()
              << std::endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    std::cerr << __FILE__ ": test fail: unknown exception caught in main"
              << std::endl;
    exit(EXIT_FAILURE);
  }
}
