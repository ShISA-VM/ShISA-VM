#include "benchmark.hpp"

#include <ShISA/Binary.hpp>
#include <ShISA/ISAModule.hpp>
#include <ShISA/Inst.hpp>

#include <limits>
#include <utility>



using benchmark::Flags;

constexpr int r0 = 0x0;
constexpr int r1 = 0x1;
constexpr int r2 = 0x2;
constexpr int r3 = 0x3;
constexpr int r4 = 0x4;
constexpr int r5 = 0x5;
constexpr int r6 = 0x6;
constexpr int r7 = 0x7;
constexpr int r8 = 0x8;
constexpr int r9 = 0x9;
constexpr int ra = 0xa;
constexpr int rb = 0xb;
constexpr int rc = 0xc;
constexpr int rd = 0xd;
constexpr int re = 0xe;
constexpr int rf = 0xf;



#define INST(op, dst, srcL, srcR)                                              \
  shisa::Inst::encode((shisa::OpCode::op), (dst), (srcL), (srcR))



static auto getBinData(benchmark::Flags flag) -> shisa::Binary::BinaryData;

static auto getISAModule(benchmark::Flags flag) -> shisa::ISAModule {
  using shisa::ISAModule;

  constexpr size_t cellsPerInst = sizeof(shisa::Inst) / sizeof(shisa::Cell);
  enum {
    N_CELLS     = std::numeric_limits<shisa::Addr>::max(),
    MAX_N_INSTS = N_CELLS / cellsPerInst
  };
  switch (flag) {
  case Flags::ONLY_NOPS:
    return std::move(ISAModule{std::move(std::vector<shisa::Inst>(
        static_cast<size_t>(MAX_N_INSTS), INST(ADD, r0, r0, r0)))});

  case Flags::ONE_LOOP:
    return std::move(ISAModule{{
        INST(ADD, rf, r0, r0),
        INST(LD, r3, rf, r0),
        INST(ADD, r2, r1, r1),
        INST(ADD, rf, rf, r2),
        INST(LD, re, rf, r0),
        INST(ADD, r4, r4, r1),
        INST(CMP, r5, r3, r4),
        INST(XOR, r5, r5, r1),
        INST(JTR, r0, r5, re),
        INST(ADD, r0, r0, r0),
    }});

  case Flags::ONE_LONG_LOOP: {
    std::vector<shisa::Inst> insts{
        INST(ADD, rf, r0, r0), INST(LD, r3, rf, r0),  INST(ADD, r2, r1, r1),
        INST(ADD, rf, rf, r2), INST(LD, re, rf, r0),  INST(ADD, r4, r4, r1),
        INST(CMP, r5, r3, r4), INST(XOR, r5, r5, r1),
    };

    constexpr size_t cellsPerData =
        sizeof(shisa::Binary::Data) / sizeof(shisa::Cell);
    constexpr size_t nData = 2;
    const size_t     nNops = MAX_N_INSTS - insts.size() - nData * cellsPerData;
    std::vector<shisa::Inst> nops(nNops, INST(ADD, r0, r0, r0));

    insts.insert(insts.end(), std::make_move_iterator(nops.begin()),
                 std::make_move_iterator(nops.end()));
    insts.emplace_back(INST(JTR, r0, r5, re));

    return std::move(ISAModule{std::move(insts)});
  }

  case Flags::NESTED_LOOPS:
    return std::move(ISAModule{{
        INST(ADD, rf, r0, r0),
        INST(LD, r3, rf, r0),
        INST(ADD, r2, r1, r1),
        INST(ADD, rf, rf, r2),
        INST(LD, re, rf, r0),
        INST(ADD, r4, r4, r1), // //
        INST(CMP, r5, r3, r4), // //
        INST(XOR, r5, r5, r1), // //
        INST(JTR, r0, r5, re), // // internal loop
        INST(ADD, r4, r0, r0), //
        INST(ADD, r6, r6, r1), //
        INST(CMP, r5, r3, r6), //
        INST(XOR, r5, r5, r1), //
        INST(JTR, r0, r5, re), // external loop
        INST(ADD, r0, r0, r0),
    }});

  case Flags::FUNCTION_IN_LOOP:
    return std::move(ISAModule{{
        INST(ADD, rf, r0, r0),  // rf = 0x0
        INST(ADD, r2, r1, r1),  //
        INST(LD, re, rf, r0),   // load loop addr
        INST(ADD, rf, rf, r2),  // rf =  0x2
        INST(LD, r6, rf, r0),   // load func addr
        INST(ADD, rf, rf, r2),  // rf =  0x4
        INST(LD, r5, rf, r0),   // load func arg addr
        INST(ADD, rf, rf, r2),  // rf =  0x6
        INST(ADD, rf, rf, r2),  // rf =  0x8
        INST(LD, r7, rf, r0),   // load func ret addr
        INST(SUB, rf, rf, r2),  // rf =  0x6
        INST(SUB, rf, rf, r2),  // rf =  0x4
        INST(ADD, r4, r4, r1),  //
        INST(ST, r0, r5, r4),   // store func arg
        INST(CALL, r6, r0, r0), // call func
        INST(LD, r8, r7, r0),   // load func ret
        INST(JTR, r0, r8, re),  // loop jump
        INST(ADD, rf, rf, r2),  // rf =  0x6
        INST(ADD, rf, rf, r2),  // rf =  0x8
        INST(ADD, rf, rf, r2),  // rf =  0xa
        INST(LD, rd, rf, r0),   // load end addr
        INST(JTR, r0, r0, rd),  // jump to end
        INST(ADD, r2, r1, r1),  // func
        INST(LD, r3, rf, r0),   // load func arg addr
        INST(LD, r3, r3, r0),   // load func arg
        INST(ADD, rf, rf, r2),  //
        INST(LD, r4, rf, r0),   // load n loops
        INST(CMP, r5, r3, r4),  //
        INST(XOR, r5, r5, r1),  //
        INST(ADD, rf, rf, r2),  //
        INST(ADD, r0, r0, r0),  //
        INST(LD, r6, rf, r0),   // load func ret addr
        INST(ST, r0, r6, r5),   // store return value
        INST(RET, r0, r0, r0),  //
        INST(ADD, r0, r0, r0),  //
    }});

  case Flags::FUNCTION_WITH_NOPS_IN_LOOP: {
    std::vector<shisa::Inst> insts{
        INST(ADD, rf, r0, r0),  // rf = 0x0
        INST(ADD, r2, r1, r1),  //
        INST(LD, re, rf, r0),   // load loop addr
        INST(ADD, rf, rf, r2),  // rf =  0x2
        INST(LD, r6, rf, r0),   // load func addr
        INST(ADD, rf, rf, r2),  // rf =  0x4
        INST(LD, r5, rf, r0),   // load func arg addr
        INST(ADD, rf, rf, r2),  // rf =  0x6
        INST(ADD, rf, rf, r2),  // rf =  0x8
        INST(LD, r7, rf, r0),   // load func ret addr
        INST(SUB, rf, rf, r2),  // rf =  0x6
        INST(SUB, rf, rf, r2),  // rf =  0x4
        INST(ADD, r4, r4, r1),  //
        INST(ST, r0, r5, r4),   // store func arg
        INST(CALL, r6, r0, r0), // call func
        INST(LD, r8, r7, r0),   // load func ret
        INST(JTR, r0, r8, re),  // loop jump
        INST(ADD, rf, rf, r2),  // rf =  0x6
        INST(ADD, rf, rf, r2),  // rf =  0x8
        INST(ADD, rf, rf, r2),  // rf =  0xa
        INST(LD, rd, rf, r0),   // load end addr
        INST(JTR, r0, r0, rd),  // jump to end
        INST(ADD, r2, r1, r1),  // func
        INST(LD, r3, rf, r0),   // load func arg addr
        INST(LD, r3, r3, r0),   // load func arg
        INST(ADD, rf, rf, r2),  //
        INST(LD, r4, rf, r0),   // load n loops
        INST(CMP, r5, r3, r4),  //
        INST(XOR, r5, r5, r1),  //
        INST(ADD, rf, rf, r2),  //
        INST(ADD, r0, r0, r0),  //
        INST(LD, r6, rf, r0),   // load func ret addr
        INST(ST, r0, r6, r5),   // store return value
    };
    std::vector<shisa::Inst> funcEnd{
        INST(RET, r0, r0, r0),
        INST(ADD, r0, r0, r0),
    };
    const size_t nNops = MAX_N_INSTS - insts.size() - funcEnd.size() -
                         getBinData(flag).size() -
                         shisa::STACK_OFFSET / cellsPerInst -
                         2 * sizeof(shisa::Reg) / sizeof(shisa::Cell);
    std::vector<shisa::Inst> nops(nNops, INST(ADD, r0, r0, r0));

    insts.insert(insts.end(), std::make_move_iterator(nops.begin()),
                 std::make_move_iterator(nops.end()));
    insts.insert(insts.end(), std::make_move_iterator(funcEnd.begin()),
                 std::make_move_iterator(funcEnd.end()));
    return std::move(ISAModule{std::move(insts)});
  }

  case Flags::FIBONACCI: {
    return std::move(ISAModule{{
        INST(ADD, rf, r0, r0),  // rf = 0x0
        INST(ADD, r2, r1, r1),  // r2 = 0x2
        INST(LD, re, rf, r0),   // load loop addr
        INST(ADD, rf, rf, r2),  // rf =  0x2
        INST(LD, r5, rf, r0),   // load n numbers
        INST(ADD, rf, rf, r2),  // rf =  0x4
        INST(LD, r6, rf, r0),   // load func addr
        INST(ADD, rf, rf, r2),  // rf =  0x6
        INST(LD, r7, rf, r0),   // load func arg addr
        INST(ADD, r4, r0, r0),  // r4 = 0x0
        INST(ST, r0, r7, r4),   // store func arg // loop start
        INST(CALL, r6, r0, r0), // call func
        INST(ADD, r4, r4, r1),  // r4 += 0x1
        INST(CMP, r8, r4, r5),  //
        INST(XOR, r8, r8, r1),  // if nth number is calculated
        //INST(ADD, r0, r0, r0),
        INST(JTR, r0, r8, re),  // loop jump
        INST(ADD, rf, rf, r2),  // rf =  0x8
        INST(ADD, rf, rf, r2),  // rf =  0xa
        INST(ADD, rf, rf, r2),  // rf =  0xc
        INST(LD, rd, rf, r0),   // load end addr
        INST(JTR, r0, r0, rd),  // jump to end
        INST(ADD, r2, r1, r1),  // func
        INST(LD, r3, rf, r0),   // load func arg addr
        INST(LD, r3, r3, r0),   // load func arg
        INST(ADD, rf, rf, r2),  // rf =  0x8
        INST(LD, re, rf, r0),   // load array address
        INST(ADD, rf, rf, r2),  // rf =  0xa
        INST(LD, rd, rf, r0),   // load jump addr if arg > 2
        INST(CMP, r4, r3, r0),  // r4 = r3 != r0
        INST(CMP, r5, r3, r1),  // r5 = r3 != r1
        INST(XOR, r6, r5, r4),   // r6 = r5 | r4
        //INST(XOR, r6, r6, r1),  //
        INST(ADD, r0, r0, r0),
        INST(JTR, r0, r6, rd),  // jump if arg > 2
        INST(MUL, r7, r3, r2),  // r7 = r3 * r2 // elem offset
        INST(ADD, r8, re, r7),  // elem addr
        INST(ST, r0, r8, r1),   // store 0x1 for i < 2
        INST(RET, r0, r0, r0),  //
        INST(MUL, r4, r3, r2),  // r4 = r3 * r2 // elem offset
        INST(ADD, r5, re, r4),  // arr[i] addr
        INST(SUB, r6, r5, r2),  // arr[i-1] addr
        INST(LD, r7, r6, r0),   // load arr[i-1]
        INST(SUB, r8, r6, r2),  // arr[i-2] addr
        INST(LD, r9, r8, r0),   // load arr[i-2]
        INST(ADD, ra, r7, r9),  // arr[i] = arr[i-1] + arr[i-2]
        INST(ST, r0, r5, ra),   // store arr[i]
        INST(RET, r0, r0, r0),  //
        INST(NOT, r0, r0, r0),  //
    }});
  }

  default:
    return std::move(ISAModule{{}});
  }
}

static auto getBinData(benchmark::Flags flag) -> shisa::Binary::BinaryData {
  using BinaryData = shisa::Binary::BinaryData;
  using Data       = shisa::Binary::Data;

  constexpr size_t cellsPerData = sizeof(Data) / sizeof(shisa::Cell);

  switch (flag) {
  case Flags::ONLY_NOPS: {
    return std::move(BinaryData{{}});
  }

  case Flags::ONE_LOOP: {
    constexpr Data nLoops   = 0xffff;
    constexpr Data loopAddr = 0x000e;
    return std::move(BinaryData{{
        nLoops,
        loopAddr,
    }});
  }

  case Flags::ONE_LONG_LOOP: {
    constexpr Data nLoops   = 0xffff;
    constexpr Data loopAddr = 0x000e;
    return std::move(BinaryData{{
        nLoops,
        loopAddr,
    }});
  }

  case Flags::NESTED_LOOPS: {
    constexpr Data nLoops   = 0x3fff;
    constexpr Data loopAddr = 0x000e;
    return std::move(BinaryData{{
        nLoops,
        loopAddr,
    }});
  }

  case Flags::FUNCTION_IN_LOOP: {
    constexpr Data loopAddr    = 0x0024;
    constexpr Data funcAddr    = 0x0038;
    constexpr Data funcArgAddr = 0x8000;
    constexpr Data nLoops      = 0xffff;
    constexpr Data funcRetAddr = 0x8002;
    constexpr Data instEnd     = 0x0050;
    return std::move(BinaryData{{
        loopAddr,    // 0x0
        funcAddr,    // 0x2
        funcArgAddr, // 0x4
        nLoops,      // 0x6
        funcRetAddr, // 0x8
        instEnd,     // 0xa
    }});
  }

  case Flags::FUNCTION_WITH_NOPS_IN_LOOP: {
    constexpr Data loopAddr    = 0x0024;
    constexpr Data funcAddr    = 0x0038;
    constexpr Data funcArgAddr = 0xfffc;
    constexpr Data nLoops      = 0xffff;
    constexpr Data funcRetAddr = 0xfffe;
    constexpr Data instEnd     = 0xeff6;
    return std::move(BinaryData{{
        loopAddr,    // 0x0
        funcAddr,    // 0x2
        funcArgAddr, // 0x4
        nLoops,      // 0x6
        funcRetAddr, // 0x8
        instEnd,     // 0xa
    }});
  }

  case Flags::FIBONACCI: {
    constexpr Data loopAddr    = 0x0022;
    constexpr Data n           = 0x7000;
    constexpr Data funcAddr    = 0x0038;
    constexpr Data funcArgAddr = 0xfffe;
    constexpr Data fibArray    = 0x10000 - cellsPerData * (n + 1);
    constexpr Data funcJmp     = 0x0058;
    constexpr Data instEnd     = 0x006c;
    return std::move(BinaryData{{
        loopAddr,    // 0x0
        n,           // 0x2
        funcAddr,    // 0x4
        funcArgAddr, // 0x6
        fibArray,    // 0x8
        funcJmp,     // 0xa
        instEnd,     // 0xc
    }});
  }


  default:
    return std::move(BinaryData{{}});
  }
}

auto benchmark::getBenchmarkBinary(Flags flag) -> shisa::Binary {
  return std::move(shisa::Binary{std::move(getISAModule(flag)),
                                 std::move(getBinData(flag))});
}
