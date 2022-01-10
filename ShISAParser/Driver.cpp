#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"

int main() {
    translator::ShISALexer* lexer = new translator::ShISALexer("ASM.txt");
    translator::ShISAParser* parser = new translator::ShISAParser(lexer);
    parser->Parse();
    parser->resolveJumps();
}
