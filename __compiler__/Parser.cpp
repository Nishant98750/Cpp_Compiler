#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <cctype>
#include <stdexcept>
#include <memory>
#include <unordered_map>

// =======================================================================
// ==               PART 1: LEXER (The Tokenizer)                       ==
// =======================================================================
// Turns the source code string into a stream of "words" or "tokens".
// This part is kept minimal as our focus is the parser.

enum class TokenType {
    // Single-character tokens
    OPEN_PAREN, CLOSE_PAREN, OPEN_BRACE, CLOSE_BRACE,
    SEMICOLON, PLUS, MINUS, STAR, SLASH,

    // One or two character tokens
    EQUAL, EQUAL_EQUAL, BANG, BANG_EQUAL,
    LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,

    // Literals & Identifiers
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    IF, ELSE, PRINT, LET,

    // Misc
    END_OF_FILE, UNKNOWN
};

struct Token {
    TokenType type;
    std::string literal;
    int line = 0;
};

// A very simple lexer function.
std::vector<Token> tokenize(std::string_view source) {
    std::vector<Token> tokens;
    size_t current = 0;
    int line = 1;

    std::unordered_map<std::string, TokenType> keywords = {
        {"if", TokenType::IF}, {"else", TokenType::ELSE},
        {"print", TokenType::PRINT}, {"let", TokenType::LET}
    };

    while (current < source.length()) {
        char c = source[current];
        size_t start = current;

        auto make_token = [&](TokenType type) {
            tokens.push_back({type, std::string(source.substr(start, current - start)), line});
        };
        auto match = [&](char expected) {
            if (current + 1 >= source.length() || source[current + 1] != expected) return false;
            current++; return true;
        };
        current++;
        switch (c) {
            case ' ': case '\r': case '\t': break;
            case '\n': line++; break;
            case '(': make_token(TokenType::OPEN_PAREN); break;
            case ')': make_token(TokenType::CLOSE_PAREN); break;
            case '{': make_token(TokenType::OPEN_BRACE); break;
            case '}': make_token(TokenType::CLOSE_BRACE); break;
            case ';': make_token(TokenType::SEMICOLON); break;
            case '+': make_token(TokenType::PLUS); break;
            case '-': make_token(TokenType::MINUS); break;
            case '*': make_token(TokenType::STAR); break;
            case '/': make_token(TokenType::SLASH); break;
            case '=': make_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
            case '!': make_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
            case '<': make_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
            case '>': make_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
            default:
                if (std::isdigit(c)) {
                    while (current < source.length() && std::isdigit(source[current])) current++;
                    make_token(TokenType::NUMBER);
                } else if (std::isalpha(c) || c == '_') {
                    while (current < source.length() && (std::isalnum(source[current]) || source[current] == '_')) current++;
                    std::string text = std::string(source.substr(start, current - start));
                    make_token(keywords.count(text) ? keywords.at(text) : TokenType::IDENTIFIER);
                } else { make_token(TokenType::UNKNOWN); }
                break;
        }
    }
    tokens.push_back({TokenType::END_OF_FILE, "", line});
    return tokens;
}


// =======================================================================
// ==       PART 2: AST (Abstract Syntax Tree) NODES                    ==
// =======================================================================
// These are simple C++ structs that will represent our code's structure.
// We use std::unique_ptr to automatically manage the memory of the tree.

// Forward-declarations are needed because the structs can refer to each other.
struct Stmt; struct Expr;

// Base struct for all statements (actions like `let`, `print`, `if`)
struct Stmt { virtual ~Stmt() = default; };

// Base struct for all expressions (things that produce a value like `5`, `x + 2`)
struct Expr { virtual ~Expr() = default; };

// An AST node for a binary operation, e.g., "left + right"
struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

// An AST node for a literal value like a number
struct LiteralExpr : Expr {
    Token value;
    explicit LiteralExpr(Token val) : value(val) {}
};

// An AST node for a variable name being used in an expression
struct VariableExpr : Expr {
    Token name;
    explicit VariableExpr(Token n) : name(n) {}
};

// An AST node for an assignment like "x = 10"
struct AssignExpr : Expr {
    Token name;
    std::unique_ptr<Expr> value;
    AssignExpr(Token n, std::unique_ptr<Expr> val) : name(n), value(std::move(val)) {}
};

// An AST node for a statement that is just an expression, e.g., "5 + 10;"
struct ExpressionStmt : Stmt {
    std::unique_ptr<Expr> expression;
    explicit ExpressionStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
};

// An AST node for a `print` command, e.g., "print x;"
struct PrintStmt : Stmt {
    std::unique_ptr<Expr> expression;
    explicit PrintStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
};

// An AST node for a `let` declaration, e.g., "let x = 10;"
struct LetStmt : Stmt {
    Token name;
    std::unique_ptr<Expr> initializer; // Can be empty if just "let x;"
    LetStmt(Token n, std::unique_ptr<Expr> init) : name(n), initializer(std::move(init)) {}
};

// An AST node for a block of statements inside { ... }
struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : statements(std::move(stmts)) {}
};

// An AST node for an `if` statement
struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // Can be empty if no else
    IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t, std::unique_ptr<Stmt> e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
};


// =======================================================================
// ==         PART 3: PARSER (The Syntax Analyzer)                      ==
// =======================================================================
// Takes the stream of tokens and builds the AST (the tree of structs).
// This is the simplest possible implementation of a recursive descent parser.

class Parser {
public:
    // Constructor: Initializes the parser with the token stream from the lexer.
    explicit Parser(const std::vector<Token>& tokens) : m_tokens(tokens) {}

    // The main entry point. It parses a list of statements until it hits the end of the file.
    std::vector<std::unique_ptr<Stmt>> parse() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (peek().type != TokenType::END_OF_FILE) {
            statements.push_back(declaration());
        }
        return statements;
    }

private:
    const std::vector<Token>& m_tokens;
    size_t m_current = 0;

    // --- Helper functions to manage the token stream ---
    Token peek() const { return m_tokens[m_current]; }
    Token previous() const { return m_tokens[m_current - 1]; }
    bool is_at_end() const { return peek().type == TokenType::END_OF_FILE; }
    Token advance() { if (!is_at_end()) m_current++; return previous(); }
    bool check(TokenType type) const { return is_at_end() ? false : peek().type == type; }
    
    // Checks if the current token is one of the given types. If so, consumes it and returns true.
    bool match(const std::vector<TokenType>& types) {
        for (TokenType type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }
    
    // Checks if the current token has a specific type. If not, it's a syntax error.
    Token consume(TokenType type, const std::string& message) {
        if (check(type)) return advance();
        throw std::runtime_error("[line " + std::to_string(peek().line) + "] Error: " + message);
    }

    // --- Parsing for Statements (Actions) ---
    // A program is a list of declarations.
    std::unique_ptr<Stmt> declaration() {
        if (match({TokenType::LET})) return let_declaration();
        return statement();
    }
    
    // Parses a 'let' statement.
    std::unique_ptr<Stmt> let_declaration() {
        Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
        std::unique_ptr<Expr> initializer = nullptr;
        if (match({TokenType::EQUAL})) {
            initializer = expression();
        }
        consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
        return std::make_unique<LetStmt>(name, std::move(initializer));
    }
    
    // The main router for all other kinds of statements.
    std::unique_ptr<Stmt> statement() {
        if (match({TokenType::IF})) return if_statement();
        if (match({TokenType::PRINT})) return print_statement();
        if (match({TokenType::OPEN_BRACE})) return std::make_unique<BlockStmt>(block());
        return expression_statement();
    }
    
    // Parses an 'if' statement. Note how it recursively calls statement() to parse its branches.
    std::unique_ptr<Stmt> if_statement() {
        consume(TokenType::OPEN_PAREN, "Expect '(' after 'if'.");
        auto condition = expression();
        consume(TokenType::CLOSE_PAREN, "Expect ')' after if condition.");
        auto thenBranch = statement();
        std::unique_ptr<Stmt> elseBranch = nullptr;
        if (match({TokenType::ELSE})) {
            elseBranch = statement();
        }
        return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }
    
    // Parses a block of statements.
    std::vector<std::unique_ptr<Stmt>> block() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!check(TokenType::CLOSE_BRACE) && !is_at_end()) {
            statements.push_back(declaration());
        }
        consume(TokenType::CLOSE_BRACE, "Expect '}' after block.");
        return statements;
    }
    
    // Parses a 'print' statement.
    std::unique_ptr<Stmt> print_statement() {
        auto value = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after value.");
        return std::make_unique<PrintStmt>(std::move(value));
    }
    
    // Parses a statement that is just an expression.
    std::unique_ptr<Stmt> expression_statement() {
        auto expr = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after expression.");
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }

    // --- Parsing for Expressions (Things that produce values) ---
    // This is the "Recursive Descent" cascade for handling operator precedence.
    // Each function handles one level of precedence and calls the next higher level.

    // Precedence Level 0: Expression (Entry Point)
    std::unique_ptr<Expr> expression() {
        return assignment();
    }
    
    // Precedence Level 1: Assignment (=)
    std::unique_ptr<Expr> assignment() {
        auto expr = equality();
        if (match({TokenType::EQUAL})) {
            Token equals = previous();
            auto value = assignment(); // Assignment is right-associative
            if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
                return std::make_unique<AssignExpr>(var->name, std::move(value));
            }
            throw std::runtime_error("[line " + std::to_string(equals.line) + "] Error: Invalid assignment target.");
        }
        return expr;
    }

    // Precedence Level 2: Equality (==, !=)
    std::unique_ptr<Expr> equality() {
        auto expr = comparison();
        while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
            Token op = previous();
            auto right = comparison();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        return expr;
    }
    
    // Precedence Level 3: Comparison (<, >, <=, >=)
    std::unique_ptr<Expr> comparison() {
        auto expr = term();
        while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
            Token op = previous();
            auto right = term();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        return expr;
    }
    
    // Precedence Level 4: Term (+, -)
    std::unique_ptr<Expr> term() {
        auto expr = factor();
        while (match({TokenType::MINUS, TokenType::PLUS})) {
            Token op = previous();
            auto right = factor();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        return expr;
    }
    
    // Precedence Level 5: Factor (*, /)
    std::unique_ptr<Expr> factor() {
        auto expr = primary();
        while (match({TokenType::SLASH, TokenType::STAR})) {
            Token op = previous();
            auto right = primary();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        return expr;
    }
    
    // Precedence Level 6: Primary (literals, variables, grouping with parentheses)
    // This is the "base case" of the expression recursion.
    std::unique_ptr<Expr> primary() {
        if (match({TokenType::NUMBER, TokenType::STRING})) {
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

        throw std::runtime_error("[line " + std::to_string(peek().line) + "] Error: Expect expression.");
    }
};


// =======================================================================
// ==         PART 4: INTERPRETER (The Program Executor)                ==
// =======================================================================
// This class "walks" the AST produced by the parser and executes the code.
// This is what makes our language actually do something!

class Interpreter {
public:
    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
        try {
            for (const auto& statement : statements) {
                execute(statement.get());
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Runtime Error: " << e.what() << std::endl;
        }
    }

private:
    // A map to store our variables. This is our program's "memory".
    std::unordered_map<std::string, double> environment;

    // Main dispatcher for statements. It checks the type of statement and calls the right handler.
    void execute(const Stmt* stmt) {
        if (auto* s = dynamic_cast<const ExpressionStmt*>(stmt)) { evaluate(s->expression.get()); }
        else if (auto* s = dynamic_cast<const PrintStmt*>(stmt)) {
            double value = evaluate(s->expression.get());
            std::cout << value << std::endl;
        }
        else if (auto* s = dynamic_cast<const LetStmt*>(stmt)) {
            double value = 0.0;
            if (s->initializer) {
                value = evaluate(s->initializer.get());
            }
            environment[s->name.literal] = value;
        }
        else if (auto* s = dynamic_cast<const BlockStmt*>(stmt)) {
            for(const auto& st : s->statements) {
                execute(st.get());
            }
        }
        else if (auto* s = dynamic_cast<const IfStmt*>(stmt)) {
            double condition = evaluate(s->condition.get());
            if (condition != 0) { // In our simple language, 0 is false, everything else is true
                execute(s->thenBranch.get());
            } else if (s->elseBranch) {
                execute(s->elseBranch.get());
            }
        }
    }
    
    // Main dispatcher for expressions. It evaluates an expression and returns its value.
    double evaluate(const Expr* expr) {
        if (auto* e = dynamic_cast<const LiteralExpr*>(expr)) {
            return std::stod(e->value.literal);
        }
        if (auto* e = dynamic_cast<const VariableExpr*>(expr)) {
            if (environment.count(e->name.literal)) {
                return environment.at(e->name.literal);
            }
            throw std::runtime_error("Undefined variable '" + e->name.literal + "'.");
        }
        if (auto* e = dynamic_cast<const AssignExpr*>(expr)) {
            double value = evaluate(e->value.get());
            if (environment.count(e->name.literal)) {
                environment[e->name.literal] = value;
                return value;
            }
            throw std::runtime_error("Undefined variable '" + e->name.literal + "'.");
        }
        if (auto* e = dynamic_cast<const BinaryExpr*>(expr)) {
            double left = evaluate(e->left.get());
            double right = evaluate(e->right.get());
            switch (e->op.type) {
                case TokenType::PLUS:          return left + right;
                case TokenType::MINUS:         return left - right;
                case TokenType::STAR:          return left * right;
                case TokenType::SLASH:         return left / right;
                case TokenType::GREATER:       return left > right;
                case TokenType::GREATER_EQUAL: return left >= right;
                case TokenType::LESS:          return left < right;
                case TokenType::LESS_EQUAL:    return left <= right;
                case TokenType::EQUAL_EQUAL:   return left == right;
                case TokenType::BANG_EQUAL:    return left != right;
                default: break;
            }
        }
        return 0.0; // Should not be reached
    }
};


// =======================================================================
// ==                    PART 5: MAIN DRIVER                            ==
// =======================================================================
// This function puts all the pieces together.

int main() {
    std::string source = R"(
        let a = 10;
        let b = 0;

        if (a > 5) {
            b = a * 2 + (a / 5); // b should become 10*2 + 2 = 22
            print b;
        } else {
            print 999; // This part should not run
        }

        let c = b - 2; // c should be 22 - 2 = 20
        print c;
    )";

    std::cout << "--- Compiling and Running SimPL Code ---\n";
    try {
        // Step 1: Lexing (Source Code -> Tokens)
        std::vector<Token> tokens = tokenize(source);

        // Step 2: Parsing (Tokens ->