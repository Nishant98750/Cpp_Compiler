// AST.h
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>
#include "Tokenizer.hpp" // We need the Token and TokenType definitions

// Forward declarations for the Visitor pattern (optional but good practice)
struct Stmt;
struct Expr;
struct FunctionStmt;
struct ReturnStmt;
struct IfStmt;
struct BlockStmt;
struct ExpressionStmt;
struct BinaryExpr;
struct UnaryExpr;
struct LiteralExpr;
struct VariableExpr;
struct AssignExpr;

// --- Visitor Interfaces ---
// Allows us to operate on the AST without modifying the AST nodes themselves.
struct StmtVisitor {
    virtual void visit(const FunctionStmt& stmt) = 0;
    virtual void visit(const ReturnStmt& stmt) = 0;
    virtual void visit(const IfStmt& stmt) = 0;
    virtual void visit(const BlockStmt& stmt) = 0;
    virtual void visit(const ExpressionStmt& stmt) = 0;
};

struct ExprVisitor {
    virtual void visit(const BinaryExpr& expr) = 0;
    virtual void visit(const UnaryExpr& expr) = 0;
    virtual void visit(const LiteralExpr& expr) = 0;
    virtual void visit(const VariableExpr& expr) = 0;
    virtual void visit(const AssignExpr& expr) = 0;
};

// --- Base Classes ---
struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) const = 0;
};

struct Expr {
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor& visitor) const = 0;
};

// --- Statement Nodes ---
struct ExpressionStmt : Stmt {
    std::unique_ptr<Expr> expression;

    ExpressionStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}

    void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : statements(std::move(stmts)) {}
    
    void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
};


struct FunctionStmt : Stmt {
    Token name;
    // For simplicity, we'll handle parameters later.
    // std::vector<Token> params; 
    std::unique_ptr<BlockStmt> body;

    FunctionStmt(Token name, std::unique_ptr<BlockStmt> body)
        : name(name), body(std::move(body)) {}

    void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch; // Can be nullptr if no else

    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> then, std::unique_ptr<Stmt> else_b)
        : condition(std::move(cond)), then_branch(std::move(then)), else_branch(std::move(else_b)) {}

    void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
};


struct ReturnStmt : Stmt {
    Token keyword;
    std::unique_ptr<Expr> value; // Can be nullptr for "return;"

    ReturnStmt(Token kw, std::unique_ptr<Expr> val)
        : keyword(kw), value(std::move(val)) {}

    void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
};

// --- Expression Nodes ---
struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}

    void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
};

struct UnaryExpr : Expr {
    Token op;
    std::unique_ptr<Expr> right;

    UnaryExpr(Token o, std::unique_ptr<Expr> r) : op(o), right(std::move(r)) {}

    void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
};

struct LiteralExpr : Expr {
    Token value;

    LiteralExpr(Token val) : value(val) {}

    void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
};

struct VariableExpr : Expr {
    Token name;

    VariableExpr(Token n) : name(n) {}

    void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
};

struct AssignExpr : Expr {
    Token name;
    std::unique_ptr<Expr> value;

    AssignExpr(Token n, std::unique_ptr<Expr> val) : name(n), value(std::move(val)) {}

    void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
};