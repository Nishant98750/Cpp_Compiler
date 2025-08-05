#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <string_view>

// It's best practice to avoid 'using namespace std;' in header files.
// We will use std:: qualification instead.

enum class TokenType {
    // --- Keywords ---
    IDENTIFIER,
    KEYWORD_FUNC,
    KEYWORD_RETURN,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_LET,       // New: for variable declaration
    KEYWORD_WHILE,     // New: while loops
    KEYWORD_FOR,       // New: for loops
    
    // --- Data Types & Booleans ---
    KEYWORD_INT,       // New: int type
    KEYWORD_STRING,    // New: string type
    KEYWORD_CHAR,      // New: char type
    KEYWORD_BOOL,      // New: bool type
    KEYWORD_TRUE,      // New: boolean true
    KEYWORD_FALSE,     // New: boolean false
    KEYWORD_NIL,       // New: null/nil value

    // --- Literals ---
    INTEGER_LITERAL,
    FLOAT_LITERAL,     // New: for floating point numbers
    STRING_LITERAL,
    CHAR_LITERAL,

    // --- Operators ---
    EQUALS,        // =
    PLUS,          // +
    MINUS,         // -
    STAR,          // *
    SLASH,         // /
    PERCENT,       // %
    CARET,         // ^

    // --- Relational & Logical Operators ---
    DBL_EQUALS,    // ==
    NOT_EQUALS,    // !=
    LESS,          // <
    GREATER,       // >
    LESS_EQ,       // <=
    GREATER_EQ,    // >=
    LOGICAL_AND,   // New: &&
    LOGICAL_OR,    // New: ||

    // --- Punctuation ---
    OPEN_PAREN,    // (
    CLOSE_PAREN,   // )
    OPEN_BRACE,    // {
    CLOSE_BRACE,   // }
    SEMICOLON,     // ;
    COMMA,         // New: ,

    // --- Misc ---
    UNKNOWN,
    END_OF_FILE
};

// Token struct now includes the line number for better error reporting.
struct Token {
    std::string literal;
    TokenType type;
    int line; // New: To track which line the token is on.
};

class Tokenizer {
public:
    // Constructor takes a string_view for efficiency.
    Tokenizer(std::string_view source)
        : m_source(source), m_line(1) { // Start at line 1
        // Initialize the keywords map once using a static block.
        if (keywords.empty()) {
            // Commands and control flow
            keywords["func"] = TokenType::KEYWORD_FUNC;
            keywords["return"] = TokenType::KEYWORD_RETURN;
            keywords["if"] = TokenType::KEYWORD_IF;
            keywords["else"] = TokenType::KEYWORD_ELSE;
            keywords["let"] = TokenType::KEYWORD_LET;
            keywords["while"] = TokenType::KEYWORD_WHILE;
            keywords["for"] = TokenType::KEYWORD_FOR;
            
            // Data types
            keywords["int"] = TokenType::KEYWORD_INT;
            keywords["string"] = TokenType::KEYWORD_STRING;
            keywords["char"] = TokenType::KEYWORD_CHAR;
            keywords["bool"] = TokenType::KEYWORD_BOOL;
            
            // Literals
            keywords["true"] = TokenType::KEYWORD_TRUE;
            keywords["false"] = TokenType::KEYWORD_FALSE;
            keywords["nil"] = TokenType::KEYWORD_NIL;
        }
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (!is_at_end()) {
            m_start = m_current;
            char c = advance();

            switch (c) {
                // Single-character tokens
                case '(': tokens.push_back(make_token(TokenType::OPEN_PAREN)); break;
                case ')': tokens.push_back(make_token(TokenType::CLOSE_PAREN)); break;
                case '{': tokens.push_back(make_token(TokenType::OPEN_BRACE)); break;
                case '}': tokens.push_back(make_token(TokenType::CLOSE_BRACE)); break;
                case ';': tokens.push_back(make_token(TokenType::SEMICOLON)); break;
                case ',': tokens.push_back(make_token(TokenType::COMMA)); break; // New
                case '+': tokens.push_back(make_token(TokenType::PLUS)); break;
                case '-': tokens.push_back(make_token(TokenType::MINUS)); break;
                case '*': tokens.push_back(make_token(TokenType::STAR)); break;
                case '%': tokens.push_back(make_token(TokenType::PERCENT)); break;
                case '^': tokens.push_back(make_token(TokenType::CARET)); break;
                
                // One or two character tokens
                case '=':
                    tokens.push_back(make_token(match('=') ? TokenType::DBL_EQUALS : TokenType::EQUALS));
                    break;
                case '!':
                    tokens.push_back(make_token(match('=') ? TokenType::NOT_EQUALS : TokenType::UNKNOWN));
                    break;
                case '<':
                    tokens.push_back(make_token(match('=') ? TokenType::LESS_EQ : TokenType::LESS));
                    break;
                case '>':
                    tokens.push_back(make_token(match('=') ? TokenType::GREATER_EQ : TokenType::GREATER));
                    break;
                case '&': // New for &&
                    if (match('&')) tokens.push_back(make_token(TokenType::LOGICAL_AND));
                    else tokens.push_back(make_token(TokenType::UNKNOWN));
                    break;
                case '|': // New for ||
                    if (match('|')) tokens.push_back(make_token(TokenType::LOGICAL_OR));
                    else tokens.push_back(make_token(TokenType::UNKNOWN));
                    break;

                // Handle comments and division
                case '/':
                    if (match('/')) { // New: Handle single-line comments
                        // A comment goes until the end of the line.
                        while (peek() != '\n' && !is_at_end()) advance();
                    } else {
                        tokens.push_back(make_token(TokenType::SLASH));
                    }
                    break;

                // Literals
                case '"': tokens.push_back(string_literal()); break;
                case '\'': tokens.push_back(char_literal()); break;
                
                // Ignore whitespace
                case ' ':
                case '\r':
                case '\t':
                    break;
                
                // Handle newlines
                case '\n':
                    m_line++;
                    break;

                default:
                    if (std::isdigit(c)) {
                        tokens.push_back(number_literal());
                    } else if (std::isalpha(c) || c == '_') {
                        tokens.push_back(identifier());
                    } else {
                        tokens.push_back(make_token(TokenType::UNKNOWN));
                    }
                    break;
            }
        }

        tokens.push_back({"", TokenType::END_OF_FILE, m_line});
        return tokens;
    }

private:
    std::string_view m_source;
    size_t m_start = 0;
    size_t m_current = 0;
    int m_line; // New: Tracks the current line number

    inline static std::unordered_map<std::string, TokenType> keywords;

    bool is_at_end() const {
        return m_current >= m_source.length();
    }

    char advance() {
        return m_source[m_current++];
    }

    char peek() const {
        if (is_at_end()) return '\0';
        return m_source[m_current];
    }
    
    char peek_next() const {
        if (m_current + 1 >= m_source.length()) return '\0';
        return m_source[m_current + 1];
    }

    bool match(char expected) {
        if (is_at_end() || m_source[m_current] != expected) {
            return false;
        }
        m_current++;
        return true;
    }

    Token make_token(TokenType type) const {
        return {std::string(m_source.substr(m_start, m_current - m_start)), type, m_line};
    }
    
    Token string_literal() {
        while (peek() != '"' && !is_at_end()) {
            if (peek() == '\n') m_line++; // Support multi-line strings
            advance();
        }
        if (is_at_end()) return make_token(TokenType::UNKNOWN); // Unterminated string
        advance(); // Consume the closing quote
        return make_token(TokenType::STRING_LITERAL);
    }
    
    Token char_literal() {
        // A char literal should ideally be one character, but we'll keep it simple
        while (peek() != '\'' && !is_at_end()) {
            advance();
        }
        if (is_at_end()) return make_token(TokenType::UNKNOWN); // Unterminated char
        advance(); // Consume the closing quote
        return make_token(TokenType::CHAR_LITERAL);
    }

    // Now supports floating point numbers
    Token number_literal() {
        while (std::isdigit(peek())) {
            advance();
        }
        
        // Look for a fractional part.
        if (peek() == '.' && std::isdigit(peek_next())) {
            // Consume the "."
            advance();
            while (std::isdigit(peek())) {
                advance();
            }
            return make_token(TokenType::FLOAT_LITERAL);
        }

        return make_token(TokenType::INTEGER_LITERAL);
    }

    Token identifier() {
        while (std::isalnum(peek()) || peek() == '_') {
            advance();
        }
        
        auto text = m_source.substr(m_start, m_current - m_start);
        auto it = keywords.find(std::string(text));
        if (it != keywords.end()) {
            return make_token(it->second); // It's a keyword
        }
        
        return make_token(TokenType::IDENTIFIER);
    }
};