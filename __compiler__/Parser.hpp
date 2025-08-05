// Parser.h
#pragma once

#include <vector>
#include <stdexcept>
#include "Tokenizer.hpp" // Use the new extended tokenizer
#include "AST.hpp"

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // The main entry point for parsing.
    std::vector<std::unique_ptr<Stmt>> parse();

private:
    // A simple exception class for parsing errors.
    class ParseError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    // --- Grammar Rule Methods ---
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> function_declaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> if_statement();
    std::vector<std::unique_ptr<Stmt>> block();
    std::unique_ptr<Stmt> return_statement();
    std::unique_ptr<Stmt> expression_statement();
    
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();

    // --- Helper Methods ---
    // Checks if the current token is one of the given types.
    bool match(const std::vector<TokenType>& types);
    // Checks the type of the current token without consuming it.
    bool check(TokenType type) const;
    // New helper for lookahead, one token into the future.
    bool check_next(TokenType type) const; // <--- THIS LINE IS NEW
    // Consumes the current token and returns it.
    Token advance();
    // Checks if we have consumed all tokens.
    bool is_at_end() const;
    // Returns the current token without consuming it.
    Token peek() const;
    // Returns the previous token.
    Token previous() const;
    // Consumes a token of a specific type or throws an error.
    Token consume(TokenType type, const std::string& message);
    // Error handling and synchronization.
    ParseError error(const Token& token, const std::string& message);
    void synchronize();

    std::vector<Token> m_tokens;
    size_t m_current = 0;
};