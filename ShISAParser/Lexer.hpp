#pragma once

#include<vector>
#include<string>
#include<fstream>
#include<regex>
#include<memory>

#include "ShISALexem.hpp"

namespace translator {

    template <typename lexem>
    class LexerBase {
    public:
        LexerBase(std::string_view prog = "") : programm(std::string(prog)), lexems(), err_str() {}

        void setProgramm(const std::string& prog) {
            programm.open(prog);
        }

        const std::vector<std::unique_ptr<lexem>>& getLexemVector() const {
            return lexems;
        }

        virtual void runLexicalParser() = 0;
        virtual ~LexerBase() = default;

        const std::string& getLexicErrors() {
            return err_str;
        }

    protected:

        std::vector<std::unique_ptr<lexem>> lexems;
        std::ifstream programm;
        std::string err_str;
    };



    class ShISALexer final : public LexerBase<SimpleLexem> {

    public:
        ShISALexer(const std::string& prog = "") : LexerBase(prog) {}

        void runLexicalParser() override {
            //TODO: Do something more smart here
            if(!programm.is_open()) {
                std::cout << "ERROR: Cannot open assembler file!" << std::endl;
                std::abort();
            }

            std::string lexem_buf;
            size_t curr_line = 1;
            while(true) {
                std::ifstream::int_type letter = programm.get();
                if (letter != ' ' && letter != '\n' && letter != ':'
                && letter != std::ifstream::traits_type::eof()) {
                    lexem_buf += static_cast<char>(letter);
                    continue;
                } else if (letter == std::ifstream::traits_type::eof()) {
                    break;
                } else if (letter == ' ' || letter == '\n' || letter == ':') {
                    if(lexem_buf == "add") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::add));
                    } else if(lexem_buf == "sub") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::sub));
                    } else if(lexem_buf == "mul") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::mul));
                    } else if(lexem_buf == "div") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::div));
                    } else if(lexem_buf == "and") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::and_op));
                    } else if(lexem_buf == "or") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::or_op));
                    } else if(lexem_buf == "not") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::not_op));
                    } else if(lexem_buf == "jtr") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::jtr));
                    } else if(lexem_buf == "ld") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::ld));
                    } else if(lexem_buf == "st") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::st));
                    } else if(lexem_buf == "cmp") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::cmp));
                    } else if(lexem_buf == "push") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::push));
                    } else if(lexem_buf == "pop") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::pop));
                    } else if(lexem_buf == "call") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::call));
                    } else if(lexem_buf == "ret") {
                        lexems.push_back(std::make_unique<OpLexem>(curr_line, ShISAInstrOpCode::ret));
                    } else if (lexem_buf == "r0") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r0));
                    } else if(lexem_buf == "r1") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r1));
                    } else if(lexem_buf == "r2") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r2));
                    } else if(lexem_buf == "r3") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r3));
                    } else if(lexem_buf == "r4") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r4));
                    } else if(lexem_buf == "r5") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r5));
                    } else if(lexem_buf == "r6") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r6));
                    } else if(lexem_buf == "r7") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r7));
                    } else if(lexem_buf == "r8") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r8));
                    } else if(lexem_buf == "r9") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r9));
                    } else if(lexem_buf == "r10") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r10));
                    } else if(lexem_buf == "r11") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r11));
                    } else if(lexem_buf == "r12") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r12));
                    } else if(lexem_buf == "r13") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r13));
                    } else if(lexem_buf == "r14") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r14));
                    } else if(lexem_buf == "r15") {
                        lexems.push_back(std::make_unique<RegLexem>(curr_line, ShISARegOpCode::r15));
                    } else if(lexem_buf == ":") {
                        lexems.push_back(std::make_unique<SimpleLexem>(ShISALexem::colon, curr_line));
                    } else {
                        if (std::regex_match(lexem_buf, std::regex("[A-Z]+"))) {
                            lexems.push_back(std::make_unique<MarkLexem>(curr_line, lexem_buf));
                        } else {
                            err_str += "LEXER_ERROR: invalid lexem: " + lexem_buf + " at line "
                                    + std::to_string(curr_line) + "\n";
                        }
                    }

                    lexem_buf.clear();

                    if(letter == '\n') { // there will be new token
                        lexems.push_back(std::make_unique<SimpleLexem>(ShISALexem::newline, curr_line));
                        ++curr_line;
                    } else if(letter == ':') {
                        lexem_buf += static_cast<char>(letter);
                    }

                }
            }
        }
    };
}