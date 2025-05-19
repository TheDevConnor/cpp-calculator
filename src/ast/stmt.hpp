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

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct ModuleStmt : public Node::Stmt {
public:
  std::string name;

  ModuleStmt(std::string name) : name(name) { kind = NodeKind::module_stmt; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "MODULE_STMT: " << name << std::endl;
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct FnStmt : public Node::Stmt {
public:
  std::string name;
  Node::Type *return_type;
  Node::Stmt *block;
  // Param vector
  const char **args;
  Node::Type **args_type;
  std::size_t size;

  FnStmt(std::string name, Node::Type *return_type,
         const std::vector<std::pair<std::string, Node::Type *>> &params,
         Node::Stmt *block, Allocator::ArenaAllocator &arena)
      : name(name), return_type(return_type), block(block),
        size(params.size()) {
    args = static_cast<const char **>(
        arena.alloc(size * sizeof(const char *), alignof(const char *)));
    args_type = static_cast<Node::Type **>(
        arena.alloc(size * sizeof(Node::Type *), alignof(Node::Type *)));

    // Properly copy each pair element to the respective arrays
    for (size_t i = 0; i < size; ++i) {
      // Allocate memory for the string and copy it
      size_t str_len = params[i].first.length() + 1; // +1 for null terminator
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
    std::cout << "    body: \n";
    block->debug(2);
    std::cout << std::endl;
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct BlockStmt : public Node::Stmt {
public:
  Node::Stmt **stmt;
  std::size_t size;

  BlockStmt(const std::vector<Node::Stmt *> &stmts,
            Allocator::ArenaAllocator &arena)
      : size(stmts.size()) {
    stmt = static_cast<Node::Stmt **>(
        arena.alloc(size * sizeof(Node::Stmt *), alignof(Node::Stmt *)));
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

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
                       std::map<std::string, llvm::Value *> &) const override;
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

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
                       std::map<std::string, llvm::Value *> &) const override;
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

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
                       std::map<std::string, llvm::Value *> &) const override;
};

/* Possable loop variations
 * loop (i = 0; i < 10) : (i++) {}
 * loop (i = 0; i < 10) {}
 * loop (i < 10) : (i++) {}
 * loop (i < 10) {}
 */

struct LoopStmt : public Node::Stmt {
  bool is_for;
  Node::Expr *init;
  Node::Expr *condition;
  Node::Expr *optional;
  Node::Stmt *block;

  LoopStmt(bool is_for, Node::Expr *init, Node::Expr *condition,
           Node::Expr *optional, Node::Stmt *block)
      : is_for(is_for), init(init), condition(condition), optional(optional),
        block(block) {
    kind = NodeKind::loop_stmt;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "LOOP_STMT: \n";
    std::cout << "     is_for: " << is_for << "\n";
    if (init != nullptr) {
      std::cout << "     init: ";
      init->debug();
    }
    if (condition != nullptr) {
      std::cout << "     condition: ";
      condition->debug();
    }
    if (optional != nullptr) {
      std::cout << "     optional: ";
      optional->debug();
    }
    if (block != nullptr) {
      std::cout << "     block: ";
      block->debug();
    }
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct PrintStmt : public Node::Stmt {
public:
  Node::Expr *fd; // file descriptor
  bool is_ln;     // is it a newline?
  Node::Expr **args;
  std::size_t size;

  PrintStmt(Node::Expr *fd, bool is_ln, const std::vector<Node::Expr *> &args,
            Allocator::ArenaAllocator &arena)
      : fd(fd), is_ln(is_ln), size(args.size()) {
    this->args = static_cast<Node::Expr **>(
        arena.alloc(size * sizeof(Node::Expr *), alignof(Node::Expr *)));
    std::copy(args.begin(), args.end(), this->args);
    kind = NodeKind::print_stmt;
  }
  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "PRINT_STMT: \n";
    std::cout << "   fd: ";
    fd->debug();
    std::cout << "\n   is_ln: " << (is_ln ? "true" : "false") << "\n";
    std::cout << "   args: \n";
    if (args != nullptr) {
      for (std::size_t i = 0; i < size; i++) {
        args[i]->debug(2);
      }
    }
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct ReturnStmt : public Node::Stmt {
public:
  Node::Expr *expr;

  ReturnStmt(Node::Expr *expr) : expr(expr) { kind = return_stmt; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "RETURN_STMT: ";
    if (expr != nullptr)
      expr->debug();
    else
      std::cout << "nullptr\n";
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
                       std::map<std::string, llvm::Value *> &) const override;
};
