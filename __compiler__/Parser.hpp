// Parser.h
#pragma once

#include <vector>
#include <stdexcept>
#include "Tokenizer.hpp" // Use the new extended tokenizer
#include "AST.hpp"

using namespace std;

class Parser {
public:
    explicit Parser(vector<Token> tokens);

    // The main entry point for parsing.
    vector<unique_ptr<Stmt>> parse();

private:
    // A simple exception class for parsing errors.
    class ParseError : public runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    // --- Grammar Rule Methods ---
    unique_ptr<Stmt> declaration();
    unique_ptr<Stmt> function_declaration();
    unique_ptr<Stmt> statement();
    unique_ptr<Stmt> if_statement();
    vector<unique_ptr<Stmt>> block();
    unique_ptr<Stmt> return_statement();
    unique_ptr<Stmt> expression_statement();
    
    unique_ptr<Expr> expression();
    unique_ptr<Expr> assignment();
    unique_ptr<Expr> equality();
    unique_ptr<Expr> comparison();
    unique_ptr<Expr> term();
    unique_ptr<Expr> factor();
    unique_ptr<Expr> unary();
    unique_ptr<Expr> primary();

    // --- Helper Methods ---
    // Checks if the current token is one of the given types.
    bool match(const vector<TokenType>& types);
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
    Token consume(TokenType type, const string& message);
    // Error handling and synchronization.
    ParseError error(const Token& token, const string& message);
    void synchronize();

    vector<Token> m_tokens;
    size_t m_current = 0;
};