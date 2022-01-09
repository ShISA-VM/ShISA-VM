#pragma once

#include "ShISAInstructionsTypes.hpp"
namespace translator {

    enum class ShISALexem {
        op =      0,
        reg =     1,
        mark =    2,
        colon =   3,
        newline = 4,
    };

    struct SimpleLexem {

        ShISALexem type;
        size_t line;

        explicit SimpleLexem(ShISALexem type_, size_t line_) : type(type_), line(line_) {}

        virtual ~SimpleLexem() = default;
    };

    struct RegLexem : public SimpleLexem {
        explicit RegLexem(size_t line_, ShISARegOpCode reg_) : SimpleLexem(ShISALexem::reg, line_), reg(reg_) {}

        ShISARegOpCode reg;
    };

    struct OpLexem : public SimpleLexem {
        explicit OpLexem(size_t line_, ShISAInstrOpCode op_) : SimpleLexem(ShISALexem::op, line_), op(op_) {}

        ShISAInstrOpCode op;
    };

    struct MarkLexem : public SimpleLexem {
        explicit MarkLexem(size_t line_, const std::string& mark_) : SimpleLexem(ShISALexem::mark, line_), mark(mark_) {}

        std::string mark;
    };


}