#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <map>

enum NodeKind {
  symbol_type,
  program,
  number,
  string,
  ident,
  binary,
  unary,
  group,
  _call,
  module_stmt,
  expr_stmt,
  var_stmt,
  return_stmt,
  fn_stmt,
  block_stmt,
  print_stmt,
};

class Node {
public:
  struct Expr {
    NodeKind kind;
    virtual void debug(int indent = 0) const = 0;
    virtual llvm::Value *
    codegen(llvm::LLVMContext &, llvm::IRBuilder<> &,
            std::map<std::string, llvm::Value *> &) const = 0;
  };

  struct Stmt {
    NodeKind kind;
    virtual void debug(int indent = 0) const = 0;
    virtual llvm::Value *
    codegen(llvm::LLVMContext &, llvm::IRBuilder<> &, llvm::Module &,
            std::map<std::string, llvm::Value *> &) const = 0;
  };

  struct Type {
    NodeKind kind;
    virtual void debug(int indent = 0) const = 0;
    virtual llvm::Type *codegen(llvm::LLVMContext &) const = 0;
  };
};

inline Node node;
