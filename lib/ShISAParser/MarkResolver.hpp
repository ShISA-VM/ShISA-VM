#pragma once

#include<unordered_map>

#include"ShISAInstructions.hpp"

namespace translator {
    template <typename instr_t>
    class MarkResolverBase {

    public:
        virtual void resolveMarks(std::vector<std::unique_ptr<instr_t>>& instr_vec) = 0;
        virtual ~MarkResolverBase() = default;
    protected:
        std::unordered_map<std::string, size_t> mark_ht;
        std::string err_str;
 };

    class ShISAMarkResolver : MarkResolverBase<ShISAOp> {
    public:
         void addMark(const std::string& mark, size_t line) {
            mark_ht[mark] = line - mark_ht.size();
         }

         void resolveMarks(std::vector<std::unique_ptr<ShISAOp>>& instr_vec) override {
            for(auto&& elt : instr_vec) {
                if(elt->op == ShISAInstrOpCode::jtr) {
                    ShISAJtr* jtr_op = dynamic_cast<ShISAJtr*>(elt.get());
                    if(mark_ht.contains(jtr_op->mark)) {
                        elt = std::make_unique<ShISAJtrResolved>(jtr_op->line, jtr_op->reg, mark_ht.at(jtr_op->mark));
                    } else {
                        std::cerr << "MARKRESOLVER_ERROR: No mark to jump at line " + std::to_string(jtr_op->line) + "\n";
                    }
                }
            }
         }

 };

}
