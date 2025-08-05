// Parser.cpp
#include "Parser.hpp"
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

// The main entry point: parse a sequence of declarations until the end of the file.
std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!is_at_end()) {
        auto stmt = declaration();
        // The parser returns nullptr on a synchronized error, filter those out.
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    return statements;
}


// --- GRAMMAR RULE IMPLEMENTATIONS ---

// declaration -> func_decl | statement ;
std::unique_ptr<Stmt> Parser::declaration() {
    try {
        if (check(TokenType::KEYWORD_FUNC) && check_next(TokenType::IDENTIFIER)) {
            // A simple lookahead to see if it's a function declaration
            advance(); // Consume 'func'
            return function_declaration();
        }
        return statement();
    } catch (ParseError& error) {
        synchronize();
        return nullptr; // Return null on error
    }
}

// func_decl -> "func" IDENTIFIER "(" ")" "{" statement* "}"
std::unique_ptr<Stmt> Parser::function_declaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    consume(TokenType::OPEN_PAREN, "Expect '(' after function name.");
    // Parameters would be handled here.
    consume(TokenType::CLOSE_PAREN, "Expect ')' after parameters.");
    consume(TokenType::OPEN_BRACE, "Expect '{' before function body.");
    auto body_statements = block();
    auto body = std::make_unique<BlockStmt>(std::move(body_statements));
    return std::make_unique<FunctionStmt>(name, std::move(body));
}


// statement -> expr_stmt | if_stmt | return_stmt | block ;
std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::KEYWORD_IF})) return if_statement();
    if (match({TokenType::KEYWORD_RETURN})) return return_statement();
    if (match({TokenType::OPEN_BRACE})) {
        return std::make_unique<BlockStmt>(block());
    }
    return expression_statement();
}

// if_stmt -> "if" "(" expression ")" statement ( "else" statement )?
std::unique_ptr<Stmt> Parser::if_statement() {
    consume(TokenType::OPEN_PAREN, "Expect '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::CLOSE_PAREN, "Expect ')' after if condition.");

    auto then_branch = statement();
    std::unique_ptr<Stmt> else_branch = nullptr;
    if (match({TokenType::KEYWORD_ELSE})) {
        else_branch = statement();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch), std::move(else_branch));
}

// block -> "{" declaration* "}"
std::vector<std::unique_ptr<Stmt>> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::CLOSE_BRACE) && !is_at_end()) {
        statements.push_back(declaration());
    }
    consume(TokenType::CLOSE_BRACE, "Expect '}' after block.");
    return statements;
}

// return_stmt -> "return" expression? ";"
std::unique_ptr<Stmt> Parser::return_statement() {
    Token keyword = previous();
    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}


// expression_statement -> expression ";"
std::unique_ptr<Stmt> Parser::expression_statement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

// expression -> assignment
std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

// assignment -> IDENTIFIER "=" assignment | equality
std::unique_ptr<Expr> Parser::assignment() {
    auto expr = equality(); // Left-hand side can be any high-precedence expression.

    if (match({TokenType::EQUALS})) {
        Token equals = previous();
        auto value = assignment(); // Right-recursive call for right-associativity.

        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = var->name;
            return std::make_unique<AssignExpr>(name, std::move(value));
        }
        error(equals, "Invalid assignment target.");
    }
    return expr;
}

// =========================================================================
// ==                      FIXED MACRO DEFINITION                         ==
// =========================================================================
// The '...' makes this a variadic macro. '__VA_ARGS__' represents all the
// extra arguments, which we wrap in braces to form the initializer list.
#define PARSE_BINARY_LEFT(higher_prec_func, ...) \
    auto expr = higher_prec_func(); \
    while (match({__VA_ARGS__})) { \
        Token op = previous(); \
        auto right = higher_prec_func(); \
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right)); \
    } \
    return expr;

// equality -> comparison ( ( "!=" | "==" ) comparison )*
std::unique_ptr<Expr> Parser::equality() {
    // Note the changed syntax: no more braces here.
    PARSE_BINARY_LEFT(comparison, TokenType::NOT_EQUALS, TokenType::DBL_EQUALS);
}

// comparison -> term ( ( ">" | ">=" | "<" | "<=" ) term )*
std::unique_ptr<Expr> Parser::comparison() {
    PARSE_BINARY_LEFT(term, TokenType::GREATER, TokenType::GREATER_EQ, TokenType::LESS, TokenType::LESS_EQ);
}

// term -> factor ( ( "-" | "+" ) factor )*
std::unique_ptr<Expr> Parser::term() {
    PARSE_BINARY_LEFT(factor, TokenType::MINUS, TokenType::PLUS);
}

// factor -> unary ( ( "/" | "*" ) unary )*
std::unique_ptr<Expr> Parser::factor() {
    PARSE_BINARY_LEFT(unary, TokenType::SLASH, TokenType::STAR, TokenType::PERCENT);
}
// =========================================================================

// unary -> ( "!" | "-" ) unary | primary
std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::MINUS})) { // Add other unary operators like ! here
        Token op = previous();
        auto right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    return primary();
}

// primary -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" | IDENTIFIER
std::unique_ptr<Expr> Parser::primary() {
    // To handle the new tokenizer:
    if (match({TokenType::KEYWORD_FALSE, TokenType::KEYWORD_TRUE, TokenType::KEYWORD_NIL,
               TokenType::INTEGER_LITERAL, TokenType::FLOAT_LITERAL,
               TokenType::STRING_LITERAL, TokenType::CHAR_LITERAL})) {
        return std::make_unique<LiteralExpr>(previous());
    }

    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous());
    }

    if (match({TokenType::OPEN_PAREN})) {
        auto expr = expression();
        consume(TokenType::CLOSE_PAREN, "Expect ')' after expression.");
        return expr;
    }
    
    throw error(peek(), "Expect expression.");
}


// --- HELPER METHOD IMPLEMENTATIONS ---

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return false;
    return peek().type == type;
}

// New helper for lookahead
bool Parser::check_next(TokenType type) const {
    if (is_at_end() || m_tokens[m_current + 1].type == TokenType::END_OF_FILE) return false;
    return m_tokens[m_current + 1].type == type;
}

Token Parser::advance() {
    if (!is_at_end()) m_current++;
    return previous();
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek() const {
    return m_tokens[m_current];
}

Token Parser::previous() const {
    return m_tokens[m_current - 1];
}

Parser::ParseError Parser::error(const Token& token, const std::string& message) {
    if (token.type == TokenType::END_OF_FILE) {
        std::cerr << "ParseError at line " << token.line << " at end: " << message << std::endl;
    } else {
        std::cerr << "ParseError at line " << token.line << " at '" << token.literal << "': " << message << std::endl;
    }
    return ParseError(message);
}

// This helps the parser recover from an error and continue parsing to find more errors.
void Parser::synchronize() {
    advance();

    while (!is_at_end()) {
        if (previous().type == TokenType::SEMICOLON) return;

        switch (peek().type) {
            case TokenType::KEYWORD_FUNC:
            case TokenType::KEYWORD_IF:
            case TokenType::KEYWORD_RETURN:
            case TokenType::KEYWORD_LET:
            case TokenType::KEYWORD_FOR:
            case TokenType::KEYWORD_WHILE:
                return;
            default:
                // Do nothing
                ;
        }
        advance();
    }
}