#include <iostream>
#include <fstream>
#include <sstream>
#include<typeinfo>
#include "Tokenizer.hpp"
#include "Parser.hpp"

// A simple main function to drive the lexer and parser.
int main() {
    // 1. Get source code from a file or string.
    std::string source = R"(
        func my_function() {
            x = 10 + 20 * 2;
            if (x > 40) {
                return x;
            }
             else {
                return 0;
            }
        }
    )";

    // 2. Tokenize the source code.
    Tokenizer tokenizer(source);
    std::vector<Token> tokens = tokenizer.tokenize();

    std::cout << "--- Tokens ---" << std::endl;
    for (const auto& token : tokens) {
        std::cout << "Literal: '" << token.literal << "'\n";
    }
    std::cout << "--------------\n\n";

    // 3. Parse the tokens into an AST.
    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> ast = parser.parse();

    // 4. (Next Step) Do something with the AST.
    // For now, we'll just confirm it parsed without crashing.
    if (!ast.empty() && ast[0] != nullptr) {
        std::cout << "Code parsed successfully into an AST!" << std::endl;
        std::cout << "The program has " << ast.size() << " top-level statement(s)." << std::endl;
    } else {
        std::cout << "Parsing failed or resulted in an empty AST." << std::endl;
    }

    return 0;
}