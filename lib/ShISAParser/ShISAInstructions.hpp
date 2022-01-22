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

        ShISAUnOp(size_t line_, const ShISAInstrOpCode op_, const ShISARegOpCode dstReg_, const ShISARegOpCode leftReg_) :
        ShISAOp(line_, op_), dstReg(dstReg_), leftReg(leftReg_) {}

        ShISARegOpCode dstReg;
        ShISARegOpCode leftReg;

    };

    struct ShISABinOp : public ShISAUnOp {

        ShISABinOp(size_t line_, const ShISAInstrOpCode op_, const ShISARegOpCode dstReg_, const ShISARegOpCode leftReg_,
              const ShISARegOpCode rightReg_) :
              ShISAUnOp(line_, op_, dstReg_, leftReg_), rightReg(rightReg_) {}

        ShISARegOpCode rightReg;
    };

    struct ShISAJtr : public ShISAOp {

        explicit ShISAJtr(size_t line_, ShISARegOpCode reg_, const std::string& mark_) :
        ShISAOp(line_, ShISAInstrOpCode::jtr), reg(reg_), mark(mark_) {}

        ShISARegOpCode reg;
        std::string mark;
    };

    struct ShISAJtrResolved : public ShISAOp {
        explicit ShISAJtrResolved(size_t line_, const ShISARegOpCode reg_, size_t idx_to_jmp_) :
        ShISAOp(line_, ShISAInstrOpCode::jtr), reg(reg_), idx_to_jmp(idx_to_jmp_) {}

        ShISARegOpCode reg;
        size_t idx_to_jmp;
    };

    struct ShISACall : public ShISAOp {
        explicit ShISACall(size_t line_, const std::string& mark_) :
        ShISAOp(line_, ShISAInstrOpCode::jtr), mark(mark_) {}

        std::string mark;
    };

    struct ShISACallResolved : public ShISAOp {
        explicit ShISACallResolved(size_t line_, size_t idx_to_jmp_) :
        ShISAOp(line_, ShISAInstrOpCode::jtr), idx_to_jmp(idx_to_jmp_) {}

        size_t idx_to_jmp;
    };
}
