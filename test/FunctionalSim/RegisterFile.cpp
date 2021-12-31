#include <FunctionalSim/FunctionalSim.hpp>
#include <ShISA/Inst.hpp>
#include <exceptions.hpp>

#include <cstdlib>
#include <iostream>
#include <string>



using shisa::Reg;
using shisa::fsim::RegisterFile;


void test_r0() {
  constexpr auto test_name = __FUNCTION__;

  RegisterFile RF;

  constexpr int r0 = 0;

  {
    const Reg r0_read = RF.read(r0);

#ifndef NDEBUG
    RF.dump(std::clog);
#endif

    constexpr Reg r0_expected = 0;

    SHISA_CHECK_TEST(r0_expected == r0_read,
                     std::string{test_name} + ": r0 == " +
                         std::to_string(r0_read) + " but must be r0 == 0");
  }

  {
    constexpr Reg some_data = 69;
    RF.write(r0, some_data);
    const Reg r0_read = RF.read(r0);

#ifndef NDEBUG
    RF.dump(std::clog);
#endif
    constexpr Reg r0_expected = 0;

    SHISA_CHECK_TEST(r0_expected == r0_read,
                     std::string{test_name} + ": r0 == " +
                         std::to_string(r0_read) + " but must be r0 == 0");
  }
}

void test_r1() {
  constexpr auto test_name = __FUNCTION__;

  RegisterFile RF;

  constexpr int r1 = 1;

  {
    const Reg r1_read = RF.read(r1);

#ifndef NDEBUG
    RF.dump(std::clog);
#endif
    constexpr Reg r1_expected = 0;

    SHISA_CHECK_TEST(r1_expected == r1_read,
                     std::string{test_name} + ": r1 == " +
                         std::to_string(r1_read) + " but must be r1 == 1");
  }

  {
    constexpr Reg some_data = 69;
    RF.write(r1, some_data);
    const Reg r1_read = RF.read(r1);

#ifndef NDEBUG
    RF.dump(std::clog);
#endif
    constexpr Reg r1_expected = 0;

    SHISA_CHECK_TEST(r1_expected == r1_read,
                     std::string{test_name} + ": r1 == " +
                         std::to_string(r1_read) + " but must be r1 == 1");
  }
}

void testReadWrite() {
  constexpr auto test_name = __FUNCTION__;

  RegisterFile RF;

#ifndef FULL_TEST
  constexpr Reg step = 0xff;
#else
  constexpr Reg step = 1;
#endif

  for (size_t r = shisa::FIRST_WRITABLE_REG; r < shisa::NREGS; r++) {
    for (Reg data = 0; data < std::numeric_limits<Reg>::max(); data += step) {
      RF.write(r, data);
      const Reg readData = RF.read(r);

#ifndef NDEBUG
      RF.dump(std::clog);
#endif
      const Reg dataExpected = data;
      SHISA_CHECK_TEST(dataExpected == readData,
                       std::string{test_name} +
                           ": written:" + std::to_string(data) +
                           " != read:" + std::to_string(readData));
    }
  }
}



int main() {
  try {
    test_r0();
    test_r1();
    testReadWrite();
  } catch (const shisa::test::Exception &e) {
    std::cerr << __FILE__ ": test fail: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}
