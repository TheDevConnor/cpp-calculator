#pragma once

#include <iostream>
#include <vector>

#include "../memory/memory.hpp"
#include "ast.hpp"

struct ProgramStmt : public Node::Stmt {
public:
  Node::Stmt **stmts;
  std::size_t size;

  ProgramStmt(const std::vector<Node::Stmt *> &list,
              Allocator::ArenaAllocator &arena)
      : size(list.size()) {
    stmts = static_cast<Node::Stmt **>(
        arena.alloc(size * sizeof(Node::Stmt *), alignof(Node::Stmt *)));
    std::copy(list.begin(), list.end(), stmts);
    kind = NodeKind::program;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    for (std::size_t i = 0; i < size; i++)
      stmts[i]->debug();
  }
};

struct ExprStmt : public Node::Stmt {
public:
  Node::Expr *expr;

  ExprStmt(Node::Expr *expr) : expr(expr) { kind = NodeKind::expr_stmt; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "EXPR_STMT: \n";
    expr->debug(1);
  }
};

struct VarStmt : public Node::Stmt {
  std::string name;
  Node::Type *type;
  Node::Expr *expr;

  VarStmt(std::string name, Node::Type *type, Node::Expr *expr)
      : name(name), type(type), expr(expr) {
    kind = var_stmt;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "VAR_STMT: \n";
    std::cout << "    name: " << name << "\n";
    std::cout << "    type: ";
    type->debug();
    std::cout << "    expr: ";
    expr->debug();
  }
};
