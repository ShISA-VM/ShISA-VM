#pragma once

#include "ShISAInstructionTypes.hpp"
namespace translator {

    enum class shisa_lexems_t {
        op =      0,
        reg =     1,
        mark =    2,
        colon =   3,
        newline = 4,
    };

    struct SimpleLexem {

        shisa_lexems_t type;
        size_t line;

        explicit SimpleLexem(shisa_lexems_t type_, size_t line_) : type(type_), line(line_) {}

        virtual ~SimpleLexem() = default;
    };

    struct RegLexem : public SimpleLexem {
        explicit RegLexem(size_t line_, ShISARegOpCode reg_) : SimpleLexem(shisa_lexems_t::reg, line_), reg(reg_) {}

        ShISARegOpCode reg;
    };

    struct OpLexem : public SimpleLexem {
        explicit OpLexem(size_t line_, ShISAInstrOpCode op_) : SimpleLexem(shisa_lexems_t::op, line_), op(op_) {}

        ShISAInstrOpCode op;
    };

    struct MarkLexem : public SimpleLexem {
        explicit MarkLexem(size_t line_, const std::string& mark_) : SimpleLexem(shisa_lexems_t::mark, line_), mark(mark_) {}

        std::string mark;
    };


}