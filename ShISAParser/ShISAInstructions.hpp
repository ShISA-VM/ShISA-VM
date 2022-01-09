#pragma once

#include "ShISAInstructionsTypes.hpp"

namespace translator {

    struct ShISAOp {

        size_t line;
        ShISAInstrOpCode op;

        explicit ShISAOp(size_t line_, const ShISAInstrOpCode op_) : line(line_), op(op_) {}
        virtual ~ShISAOp() = default;
    };


    //TODO: проверки на типы параметров
    struct ShISAUnOp : public ShISAOp {

        ShISAUnOp(size_t line_, const ShISAInstrOpCode op_, const ShISARegOpCode leftReg_, const ShISARegOpCode dstReg_) :
        ShISAOp(line_, op_), leftReg(leftReg_), dstReg(dstReg_) {}

        ShISARegOpCode dstReg;
        ShISARegOpCode leftReg;

    };

    struct ShISABinOp : public ShISAUnOp {

        ShISABinOp(size_t line_, const ShISAInstrOpCode op_, const ShISARegOpCode leftReg_,
              const ShISARegOpCode rightReg_, const ShISARegOpCode dstReg_) :
              ShISAUnOp(line_, op_, leftReg_, dstReg_), rightReg(rightReg_) {}

        ShISARegOpCode rightReg;
    };

    struct ShISAJtr : public ShISAOp {

        explicit ShISAJtr(size_t line_, ShISARegOpCode reg_, const std::string& mark_) : ShISAOp(line_, ShISAInstrOpCode::jtr), reg(reg_), mark(mark_) {}

        ShISARegOpCode reg;
        std::string mark;
    };

    struct ShISAJtrResolved : public ShISAOp {
        explicit ShISAJtrResolved(size_t line_, const ShISARegOpCode reg_, size_t idx_to_jmp_) : ShISAOp(line_, ShISAInstrOpCode::jtr), reg(reg_), idx_to_jmp(idx_to_jmp_) {}

        ShISARegOpCode reg;
        size_t idx_to_jmp;
    };
}