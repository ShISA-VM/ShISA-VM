#pragma once

namespace translator {
    enum class ShISAInstrOpCode {
        add = 0x01,
        sub  = 0x02,
        mul  = 0x03,
        div  = 0x04,
        and_op  = 0x05,
        or_op   = 0x06,
        cmp  = 0x07,
        not_op  = 0x08,
        jtr  = 0x09,
        ld   = 0x0a,
        st   = 0x0b,
        push = 0x0c,
        pop  = 0x0d,
        call = 0x0e,
        ret  = 0x0f,
    };

    enum class ShISARegOpCode {
        r0  = 0x00,
        r1  = 0x01,
        r2  = 0x02,
        r3  = 0x03,
        r4  = 0x04,
        r5  = 0x05,
        r6  = 0x06,
        r7  = 0x07,
        r8  = 0x08,
        r9  = 0x09,
        r10 = 0x0a,
        r11 = 0x0b,
        r12 = 0x0c,
        r13 = 0x0d,
        r14 = 0x0e,
        r15 = 0x0f,
    };
}
