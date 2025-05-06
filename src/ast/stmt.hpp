#pragma once

#include <cstddef>
#include <cstring>
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

struct FnStmt : public Node::Stmt {
 public:
  std::string name;
  Node::Type *return_type;
  // Param vector
  const char **args;
  Node::Type **args_type;
  std::size_t size;

  FnStmt(std::string name, Node::Type *return_type, const std::vector<std::pair<std::string, Node::Type *>> &params, Allocator::ArenaAllocator &arena)
      : name(name), return_type(return_type), size(params.size()) {
    args = static_cast<const char **>(arena.alloc(size * sizeof(const char *), alignof(const char *)));
    args_type = static_cast<Node::Type **>(arena.alloc(size * sizeof(Node::Type *), alignof(Node::Type *)));

    // Properly copy each pair element to the respective arrays
    for (size_t i = 0; i < size; ++i) {
      // Allocate memory for the string and copy it
      size_t str_len = params[i].first.length() + 1;  // +1 for null terminator
      char *str_copy = static_cast<char *>(arena.alloc(str_len, alignof(char)));
      std::strcpy(str_copy, params[i].first.c_str());

      // Store pointer to the copied string
      args[i] = str_copy;

      // Store the type pointer directly
      args_type[i] = params[i].second;
    }

    kind = NodeKind::fn_stmt;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "FN_STMT: \n";
    std::cout << "   name: " << name << "\n";
    std::cout << "   type: ";
    return_type->debug();
    std::cout << "\n   params: \n";
    if (args != nullptr && args_type != nullptr) {
      for (std::size_t i = 0; i < size; i++) {
        std::cout << "    {" << args[i] << " ";
        args_type[i]->debug();
        std::cout << "}\n";
      }
    }
    std::cout << std::endl;
  }
};

struct BlockStmt : public Node::Stmt {
 public:
  Node::Stmt **stmt;
  std::size_t size;

  BlockStmt(const std::vector<Node::Stmt *> &stmts, Allocator::ArenaAllocator &arena) : size(stmts.size()) {
    stmt = static_cast<Node::Stmt **>(arena.alloc(size * sizeof(Node::Stmt *), alignof(Node::Stmt *)));
    std::copy(stmts.begin(), stmts.end(), stmt);
    kind = block_stmt;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "BLOCK: \n";
    if (stmt != nullptr) {
      for (std::size_t i = 0; i < size; i++)
        stmt[i]->debug(1);
    }
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
 public:
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
    std::cout << "\n    expr: ";
    expr->debug();
  }
};

struct ReturnStmt : public Node::Stmt {
 public:
  Node::Expr *expr;

  ReturnStmt(Node::Expr *expr) : expr(expr) {
    kind = return_stmt;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "RETURN_STMT: ";
    if (expr != nullptr)
      expr->debug();
    else
      std::cout << "nullptr\n";
  }
};
