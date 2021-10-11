#pragma once

#include<string>
#include<vector>
#include<algorithm>

#include "Lexer.hpp"
#include "ShISAInstructions.hpp"
#include "MarkResolver.hpp"

namespace translator {

    template <typename lexer_t, typename instr_t>
    class ParserBase {
    public:
        ParserBase(lexer_t* lexer_ = nullptr): lexer(lexer_), err_str(), operations() {}

        virtual void runGrammarParser() = 0;
        virtual void resolveJumps() = 0;
        virtual ~ParserBase() = default;

    protected:
        std::string err_str;
        lexer_t* lexer;
        std::vector<std::unique_ptr<instr_t>> operations;
    };



    class ShISAParser : public ParserBase<ShISALexer, ShISAOp> {

    public:
        ShISAParser() : ParserBase<ShISALexer, ShISAOp>() {}
        
        void runGrammarParser() override {
            
            const std::vector<std::unique_ptr<SimpleLexem>>& lexems = lexer->getLexemVector();
            std::unique_ptr<ShISAOp> buf_op{nullptr};
            for(size_t pc = 0; pc < lexems.size(); ++pc) {

                shisa_lexems_t curr_lexem_type = lexems.at(pc)->type;

                if(curr_lexem_type == shisa_lexems_t::op) {
                    OpLexem* elt = dynamic_cast<OpLexem*>(lexems.at(pc).get());
                    switch(elt->op) {
                        case ShISAInstrOpCode::add:
                        case ShISAInstrOpCode::sub:
                        case ShISAInstrOpCode::mul:
                        case ShISAInstrOpCode::div:
                        case ShISAInstrOpCode::and_op:
                        case ShISAInstrOpCode::or_op:
                        case ShISAInstrOpCode::cmp:
                            if(pc+3 < lexems.size()) {
                                const std::vector<std::pair<shisa_lexems_t, shisa_lexems_t>> args{{lexems.at(pc+1).get()->type, shisa_lexems_t::reg},
                                                                                            {lexems.at(pc+2).get()->type, shisa_lexems_t::reg},
                                                                                            {lexems.at(pc+3).get()->type, shisa_lexems_t::reg}};
                                //TODO: maybe do smth better
                                if(checkArgs(args, pc, elt->line)) {
                                    if((pc+4) == lexems.size() || lexems.at(pc+4)->type == shisa_lexems_t::newline) {
                                        operations.push_back(
                                                std::make_unique<ShISABinOp>(elt->line, elt->op, dynamic_cast<RegLexem*>(lexems.at(pc+1).get())->reg,
                                                                             dynamic_cast<RegLexem*>(lexems.at(pc+2).get())->reg, dynamic_cast<RegLexem*>(lexems.at(pc+3).get())->reg));
                                        pc += 4;
                                    } else {
                                        err_str += "GRAMMAR_ERROR: Too many arguments given at line " + std::to_string(elt->line) + "\n";
                                        skipToNewline(pc);
                                    }
                                }
                            } else {
                                err_str += "GRAMMAR_ERROR: End of programm, but programm receive too few arguments at line "
                                        + std::to_string(elt->line) + "\n";
                                pc += 4;
                            }
                            break;
                        case ShISAInstrOpCode::not_op:
                        case ShISAInstrOpCode::ld:
                        case ShISAInstrOpCode::st:
                        case ShISAInstrOpCode::push:
                        case ShISAInstrOpCode::pop:
                            if(pc+2 < lexems.size()) {
                                const std::vector<std::pair<shisa_lexems_t, shisa_lexems_t>> args{{lexems.at(pc+1).get()->type, shisa_lexems_t::reg},
                                                                                                  {lexems.at(pc+2).get()->type, shisa_lexems_t::reg}};
                                if(checkArgs(args, pc, elt->line)) {
                                    if((pc+3) == lexems.size() || lexems.at(pc+3)->type == shisa_lexems_t::newline) {
                                        operations.push_back(
                                                std::make_unique<ShISAUnOp>(elt->line, elt->op, dynamic_cast<RegLexem*>(lexems.at(pc+1).get())->reg,
                                                                            dynamic_cast<RegLexem*>(lexems.at(pc+2).get())->reg));
                                        pc += 3;
                                    } else {
                                        err_str += "GRAMMAR_ERROR: Too many arguments given at line " + std::to_string(elt->line) +
                                                "\n";
                                        skipToNewline(pc);
                                    }
                                }
                            } else {
                                err_str += "GRAMMAR_ERROR: End of programm, but programm receive too few arguments at line "
                                        + std::to_string(elt->line) + "\n";
                                pc += 3;
                            }
                        case ShISAInstrOpCode::jtr:
                            //TODO: move this algorithm to function and reuse
                            if(pc+2 < lexems.size()) {
                                const std::vector<std::pair<shisa_lexems_t, shisa_lexems_t>> args{{lexems.at(pc+1).get()->type, shisa_lexems_t::reg},
                                                                                                  {lexems.at(pc+2).get()->type, shisa_lexems_t::mark}};
                                if(checkArgs(args, pc, elt->line)) {
                                    if((pc+3) == lexems.size() || lexems.at(pc+3)->type == shisa_lexems_t::newline) {
                                        operations.push_back(
                                                std::make_unique<ShISAJtr>(elt->line, dynamic_cast<RegLexem*>(lexems.at(pc+1).get())->reg,
                                                                            dynamic_cast<MarkLexem*>(lexems.at(pc+2).get())->mark));
                                        pc += 3;
                                    } else {
                                        err_str += "GRAMMAR_ERROR: Too many arguments given at line " + std::to_string(elt->line) +
                                                "\n";
                                        skipToNewline(pc);
                                    }
                                }
                            } else {
                                err_str += "GRAMMAR_ERROR: End of programm, but programm receive too few arguments at line "
                                        + std::to_string(elt->line) + "\n";
                                pc += 3;
                            }
                        case ShISAInstrOpCode::call:
                            if(pc+1 < lexems.size()) {
                                const std::vector<std::pair<shisa_lexems_t, shisa_lexems_t>> args{{lexems.at(pc+1).get()->type, shisa_lexems_t::reg}};
                                if(checkArgs(args, pc, elt->line)) {
                                    if((pc+2) == lexems.size() || lexems.at(pc+2)->type == shisa_lexems_t::newline) {
                                        operations.push_back(
                                                std::make_unique<ShISAJtr>(elt->line, ShISARegOpCode::r1, dynamic_cast<MarkLexem*>(lexems.at(pc+1).get())->mark));
                                        pc += 2;
                                    } else {
                                        err_str += "GRAMMAR_ERROR: Too many arguments given at line " + std::to_string(elt->line) +
                                                "\n";
                                        skipToNewline(pc);
                                    }
                                }
                            } else {
                                err_str += "GRAMMAR_ERROR: End of programm, but programm receive too few arguments at line "
                                        + std::to_string(elt->line) + "\n";
                                pc += 2;
                            }
                        case ShISAInstrOpCode::ret:
                            if((pc+1) == lexems.size() || lexems.at(pc+1)->type == shisa_lexems_t::newline) {
                                operations.push_back(
                                        std::make_unique<ShISAOp>(elt->line, ShISAInstrOpCode::ret));
                                pc += 1;
                            } else {
                                err_str += "GRAMMAR_ERROR: Too many arguments given at line " + std::to_string(elt->line) +
                                        "\n";
                                skipToNewline(pc);
                            }
                    }
                } else if(curr_lexem_type == shisa_lexems_t::reg) {
                    err_str += "GRAMMAR_ERROR: Unexpected register at line" + std::to_string(lexems.at(pc).get()->line) + "\n";
                    ++pc;
                } else if(curr_lexem_type == shisa_lexems_t::newline) {
                    //TODO: mb in one line
                    ++pc;
                } else if(curr_lexem_type == shisa_lexems_t::mark) {
                    if(pc+2 < lexems.size()) {
                        if(lexems.at(pc+1).get()->type == shisa_lexems_t::colon && lexems.at(pc+2).get()->type == shisa_lexems_t::newline) {
                            markResolver.addMark(dynamic_cast<MarkLexem*>(lexems.at(pc).get())->mark, lexems.at(pc).get()->line);
                            pc += 2;
                        } else if(lexems.at(pc+1).get()->type != shisa_lexems_t::colon) {
                            err_str += "GRAMMAR_ERROR: Unexpected mark at line " + std::to_string(lexems.at(pc).get()->line) + "\n";
                            ++pc;
                        }
                    } else {
                        err_str += "GRAMMAR_ERROR: Expected newline after mark at line " + std::to_string(lexems.at(pc).get()->line) + "\n";
                        ++pc;
                    }
                } else if(curr_lexem_type == shisa_lexems_t::colon) {
                    err_str += "GRAMMAR_ERROR: Unexpected colon at line " + std::to_string(lexems.at(pc).get()->line) + "\n";
                    ++pc;
                } else {
                    //TODO: do smth more smart
                    err_str += "Some shit happens\n";
                    std::abort();
                }
            }
        }

        void resolveJumps() override {
            markResolver.resolveMarks(operations);
        }

    private:

        int checkArgs(const std::vector<std::pair<shisa_lexems_t, shisa_lexems_t>>& args, size_t& pc, size_t line) {
            //TODO: maybe do smth better
            if(std::all_of(args.begin(), args.end(), [](std::pair<shisa_lexems_t, shisa_lexems_t> arg) {return arg.first == arg.second;})) {
                return true;
            } else if(std::any_of(args.begin(), args.end(), [](std::pair<shisa_lexems_t, shisa_lexems_t> arg) {return arg.first == shisa_lexems_t::newline;})) {
                //TODO:  check if arguments before newline valid
                err_str += "GRAMMAR_ERROR: Too few arguments at line " + std::to_string(line) + "\n";
                //TODO: do smth more smart
                skipToNewline(pc);
                return false;
            } else {
                err_str += "GRAMMAR_ERROR: invalid arguments at line " + std::to_string(line) + "\n";
                pc += args.size() + 1; // move pc to lexem right after arguments
                return false;
            }
        }

        void skipToNewline(size_t& pc) {
            while(lexer->getLexemVector().at(pc)->type != shisa_lexems_t::newline) {
                ++pc;
            }
        }

        ShISAMarkResolver markResolver;

    };


};