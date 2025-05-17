#pragma once

#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <string>

#include "ast.hpp"

struct Number : public Node::Expr {
public:
  std::string value;

  Number(std::string value) : value(value) { kind = NodeKind::number; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "Number Node: " << value << std::endl;
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct Ident : public Node::Expr {
  std::string ident;

  Ident(std::string ident) : ident(ident) { kind = NodeKind::ident; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "Ident Node: " << ident << std::endl;
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct String : public Node::Expr {
public:
  std::string value;

  String(std::string value) : value(value) { kind = NodeKind::string; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "String Node: " << value << std::endl;
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct Binary : public Node::Expr {
public:
  Node::Expr *left;
  Node::Expr *right;
  std::string op;

  Binary(Node::Expr *left, Node::Expr *right, std::string op)
      : left(left), right(right), op(op) {
    kind = NodeKind::binary;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "Binary Node: " << std::endl;
    std::cout << "     op: " << op << std::endl;
    if (left == nullptr) {
    } else {
      std::cout << "   left: \n\t";
      left->debug();
    }
    if (right == nullptr) {
    } else {
      std::cout << "  right: \n\t";
      right->debug();
    }
    std::cout << std::endl;
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct Unary : public Node::Expr {
public:
  Node::Expr *right;
  std::string op;

  Unary(Node::Expr *right, std::string op) : right(right), op(op) {
    kind = NodeKind::unary;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "Unary Node: " << std::endl;
    std::cout << "      op: " << op << std::endl;
    std::cout << "   right: ";
    right->debug();
    std::cout << std::endl;
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct Group : public Node::Expr {
public:
  Node::Expr *expr;

  Group(Node::Expr *expr) : expr(expr) { kind = NodeKind::group; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "Group: \n\t";
    expr->debug();
    std::cout << std::endl;
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &,
                       std::map<std::string, llvm::Value *> &) const override;
};

struct Call : public Node::Expr {
public:
  Node::Expr *name;
  std::vector<Node::Expr *> args;

  Call(Node::Expr *name, std::vector<Node::Expr *> args)
      : name(name), args(args) {
    kind = NodeKind::_call;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "Call: " << name << std::endl;
    for (auto arg : args) {
      arg->debug();
    }
  }

  llvm::Value *codegen(llvm::LLVMContext &, llvm::IRBuilder<> &,
                       std::map<std::string, llvm::Value *> &) const override;
};
