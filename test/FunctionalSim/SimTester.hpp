#pragma once

#include <FunctionalSim/Sim.hpp>
#include <FunctionalSim/isSim.hpp>

#include <concepts>
#include <iostream>
#include <stdexcept>



/* clang-format off */ #define ever ;; /* clang-format on */



namespace shisa::test {

template <typename reg_t, typename addr_t, typename cell_t, size_t n_regs,
          template <typename Reg, typename Addr, typename Cell, size_t NRegs>
          class Sim_>
requires(std::unsigned_integral<reg_t> &&std::unsigned_integral<addr_t>
             &&std::unsigned_integral<cell_t> &&shisa::fsim::isSim<
                 Sim_<reg_t, addr_t, cell_t, n_regs>>) class SimTester {
public:
  using Reg  = reg_t;
  using Addr = addr_t;
  using Cell = cell_t;
  using Sim  = Sim_<Reg, Addr, Cell, shisa::NREGS>;

  SimTester()                  = delete;
  SimTester(const SimTester &) = delete;

  static void testArithmetic() { // {{{
    std::string test_name = __FUNCTION__;
    test_name.append(": opcode ");

    auto arithmeticCases = {
        std::pair{shisa::OpCode::ADD, 0x0001},
        std::pair{shisa::OpCode::SUB, 0xffff},
        std::pair{shisa::OpCode::MUL, 0x0000},
        std::pair{shisa::OpCode::DIV, 0x0000},
        std::pair{shisa::OpCode::AND, 0x0000},
        std::pair{shisa::OpCode::OR, 0x0001},
        std::pair{shisa::OpCode::NOT, 0xffff},
        std::pair{shisa::OpCode::CMP, 0xffff},
    };

    for (const auto [testingOpCode, expectedRegValue] :
         std::views::all(arithmeticCases)) {
      for (const auto r :
           std::views::iota(shisa::FIRST_WRITABLE_REG, shisa::NREGS)) {
        constexpr Reg r0      = 0;
        constexpr Reg r1      = 1;
        Inst::RawInst addInst = Inst::encode(testingOpCode, r, r0, r1);
        ISAModule     M{{addInst}};
        const Binary  bin{std::move(M), {}};
        Sim           sim(bin);

        sim.execute();
        const auto &state    = sim.getState();
        const Reg   regValue = state.readReg(r);
        SHISA_CHECK_TEST(expectedRegValue == regValue,
                         std::string{test_name} +
                             std::to_string(static_cast<int>(testingOpCode)) +
                             ": r" + std::to_string(r) +
                             " == " + std::to_string(regValue) +
                             " but must be r" + std::to_string(r) +
                             " == " + std::to_string(expectedRegValue));
      }
    }
  } // }}}

  static void testJump() { // {{{
    constexpr auto test_name = __FUNCTION__;

    ISAModule M{{
        Inst::encode(shisa::OpCode::ADD, 0x2, 0x1, 0x1),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0x0, 0x0),
        Inst::encode(shisa::OpCode::LD, 0x3, 0xf, 0x0),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0xf, 0x2),
        Inst::encode(shisa::OpCode::LD, 0x4, 0xf, 0x0),
        Inst::encode(shisa::OpCode::JTR, 0x0, 0x1, 0x3),
        Inst::encode(shisa::OpCode::ADD, 0x5, 0x0, 0x1),
        Inst::encode(shisa::OpCode::JTR, 0x0, 0x0, 0x4),
        Inst::encode(shisa::OpCode::ADD, 0x6, 0x0, 0x1),
        Inst::encode(shisa::OpCode::ADD, 0x0, 0x0, 0x0),
        Inst::encode(shisa::OpCode::ADD, 0x0, 0x0, 0x0),
        Inst::encode(shisa::OpCode::ADD, 0x7, 0x0, 0x1),
        Inst::encode(shisa::OpCode::ADD, 0x0, 0x0, 0x0),
    }};

    Binary::BinaryData data = {0x0014, 0x0018};

    const Binary bin{std::move(M), std::move(data)};
    Sim          sim{bin};

    try {
      /* clang-format off */ #define ever ;; /* clang-format on */
      for (ever) {
        sim.execute();
      }
    } catch (const shisa::ProgramEnd &e) {
    }

    auto testCases = {
        std::pair(0x5, 0x1),
        std::pair(0x6, 0x0),
        std::pair(0x7, 0x1),
    };

    const auto &state = sim.getState();
    for (const auto [r, expectedRegValue] : testCases) {
      const Reg regValue = state.readReg(r);
      SHISA_CHECK_TEST(expectedRegValue == regValue,
                       std::string{test_name} + ": r" + std::to_string(r) +
                           " == " + std::to_string(regValue) +
                           " but must be r" + std::to_string(r) +
                           " == " + std::to_string(expectedRegValue));
    }
  } // }}}

  static void testMemory() { // {{{
    constexpr auto test_name = __FUNCTION__;

    ISAModule M{{
        Inst::encode(shisa::OpCode::ADD, 0x2, 0x1, 0x1),
        Inst::encode(shisa::OpCode::ADD, 0x3, 0x2, 0x1),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0x0, 0x0),
        Inst::encode(shisa::OpCode::LD, 0x4, 0xf, 0x0),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0xf, 0x2),
        Inst::encode(shisa::OpCode::LD, 0x5, 0xf, 0x0),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0xf, 0x2),
        Inst::encode(shisa::OpCode::LD, 0x6, 0xf, 0x0),
        Inst::encode(shisa::OpCode::ST, 0x0, 0xf, 0x4),
        Inst::encode(shisa::OpCode::LD, 0x7, 0xf, 0x0),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0xf, 0x2),
        Inst::encode(shisa::OpCode::LD, 0x8, 0xf, 0x0),
        Inst::encode(shisa::OpCode::ST, 0x0, 0xf, 0x7),
        Inst::encode(shisa::OpCode::LD, 0x9, 0xf, 0x0),
        Inst::encode(shisa::OpCode::PUSH, 0x0, 0x5, 0x0),
        Inst::encode(shisa::OpCode::POP, 0xa, 0x0, 0x0),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0xf, 0x2),
        Inst::encode(shisa::OpCode::MUL, 0xe, 0xf, 0xf),
        Inst::encode(shisa::OpCode::MUL, 0xe, 0xe, 0x2),
        Inst::encode(shisa::OpCode::ST, 0x0, 0xe, 0xe),
        Inst::encode(shisa::OpCode::LD, 0xb, 0xe, 0x0),
    }};

    Binary::BinaryData data = {0xbeef, 0xdead, 0xeeee};

    const Binary bin{std::move(M), std::move(data)};
    Sim          sim{bin};

    try {
      for (ever) {
        sim.execute();
      }
    } catch (const shisa::ProgramEnd &e) {
    }

    const auto testCases = {
        std::pair(0x2, 0x0002), std::pair(0x3, 0x0003), std::pair(0x4, 0xbeef),
        std::pair(0x5, 0xdead), std::pair(0x6, 0xeeee), std::pair(0x7, 0xeeee),
        std::pair(0x8, 0x1211), std::pair(0x9, 0x1211), std::pair(0xa, 0xdead),
        std::pair(0xb, 0x0080), std::pair(0xc, 0x0000), std::pair(0xd, 0x0000),
        std::pair(0xe, 0x0080), std::pair(0xf, 0x0008),
    };

    const auto &state = sim.getState();
    for (const auto [r, expectedRegValue] : testCases) {
      const Reg regValue = state.readReg(r);
      SHISA_CHECK_TEST(expectedRegValue == regValue,
                       std::string{test_name} + ": r" + std::to_string(r) +
                           " == " + std::to_string(regValue) +
                           " but must be r" + std::to_string(r) +
                           " == " + std::to_string(expectedRegValue));
    }
  } // }}}

  static void testFuncs() { // {{{
    constexpr auto test_name = __FUNCTION__;

    ISAModule M{{
        Inst::encode(shisa::OpCode::ADD, 0x2, 0x1, 0x1),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0x0, 0x0),
        Inst::encode(shisa::OpCode::LD, 0x3, 0xf, 0x0),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0xf, 0x2),
        Inst::encode(shisa::OpCode::LD, 0x4, 0xf, 0x0),
        Inst::encode(shisa::OpCode::CALL, 0x3, 0x0, 0x0),
        Inst::encode(shisa::OpCode::LD, 0x5, 0x4, 0x0),
        Inst::encode(shisa::OpCode::ADD, 0x6, 0x5, 0x1),
        Inst::encode(shisa::OpCode::ADD, 0xf, 0xf, 0x2),
        Inst::encode(shisa::OpCode::LD, 0x7, 0xf, 0x0),
        Inst::encode(shisa::OpCode::JTR, 0x0, 0x0, 0x7),
        Inst::encode(shisa::OpCode::MUL, 0x4, 0x2, 0x2),
        Inst::encode(shisa::OpCode::MUL, 0x4, 0x4, 0x4),
        Inst::encode(shisa::OpCode::LD, 0x3, 0xf, 0x0),
        Inst::encode(shisa::OpCode::ST, 0x0, 0x3, 0x4),
        Inst::encode(shisa::OpCode::RET, 0x0, 0x0, 0x0),
    }};

    Binary::BinaryData data = {0x001c, 0x2000, 0x0026};

    const Binary bin{std::move(M), std::move(data)};
    Sim          sim{bin};

    try {
      for (ever) {
        sim.execute();
      }
    } catch (const shisa::ProgramEnd &e) {
    }

    const auto testCases = {
        std::pair(0x2, 0x0002), std::pair(0x3, 0x001c), std::pair(0x4, 0x2000),
        std::pair(0x5, 0x0010), std::pair(0x6, 0x0011), std::pair(0x7, 0x0026),
        std::pair(0x8, 0x0000), std::pair(0x9, 0x0000), std::pair(0xa, 0x0000),
        std::pair(0xb, 0x0000), std::pair(0xc, 0x0000), std::pair(0xd, 0x0000),
        std::pair(0xe, 0x0000), std::pair(0xf, 0x0004),
    };

    const auto &state = sim.getState();
    for (const auto [r, expectedRegValue] : testCases) {
      const Reg regValue = state.readReg(r);
      SHISA_CHECK_TEST(expectedRegValue == regValue,
                       std::string{test_name} + ": r" + std::to_string(r) +
                           " == " + std::to_string(regValue) +
                           " but must be r" + std::to_string(r) +
                           " == " + std::to_string(expectedRegValue));
    }
  } // }}}

  static void runTests() {
    try {
      testArithmetic();
      testJump();
      testMemory();
      testFuncs();
    } catch (const shisa::test::Exception &e) {
      std::cerr << __FILE__ ": test fail: " << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (const std::exception &e) {
      std::cerr << __FILE__ ": test fail: exception caught in main: "
                << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (...) {
      std::cerr << __FILE__ ": test fail: unknown exception caught in main"
                << std::endl;
      exit(EXIT_FAILURE);
    }
  }
};

} // namespace shisa::test
