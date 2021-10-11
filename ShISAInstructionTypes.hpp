#pragma once

namespace translator {
    enum class ShISAInstrOpCode {
        add = 0x0000,
        sub  = 0x0001,
        mul  = 0x0010,
        div  = 0x0011,
        and_op  = 0x0100,
        or_op   = 0x0101,
        not_op  = 0x0110,
        jmp  = 0x0111,
        jtr  = 0x1000,
        ld   = 0x1001,
        st   = 0x1010,
        cmp  = 0x1011,
        push = 0x1100,
        pop  = 0x1101,
        call = 0x1110,
        ret  = 0x1111,
    };

    enum class ShISARegOpCode {
        r0  = 0x0000,
        r1  = 0x0001,
        r2  = 0x0010,
        r3  = 0x0011,
        r4  = 0x0100,
        r5  = 0x0101,
        r6  = 0x0110,
        r7  = 0x0111,
        r8  = 0x1000,
        r9  = 0x1001,
        r10 = 0x1010,
        r11 = 0x1011,
        r12 = 0x1100,
        r13 = 0x1101,
        r14 = 0x1110,
        r15 = 0x1111,
    };
}
