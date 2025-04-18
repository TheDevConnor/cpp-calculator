#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "../memory/memory.hpp"
#include "ast.hpp"

struct ProgramExpr : public Node::Expr {
public:
  Node::Expr **exprs;
  std::size_t size;

  ProgramExpr(const std::vector<Node::Expr *> &list,
              Allocator::ArenaAllocator &arena)
      : size(list.size()) {
    exprs = static_cast<Node::Expr **>(arena.alloc(size * sizeof(Node::Expr *), alignof(Node::Expr *)));
    std::copy(list.begin(), list.end(), exprs);
    kind = NodeKind::program;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    for (std::size_t i = 0; i < size; i++)
      exprs[i]->debug();
  }

  double eval() const override;
};

struct Number : public Node::Expr {
public:
  std::string value;

  Number(std::string value) : value(value) { kind = NodeKind::number; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "Number Node: " << value << std::endl;
  }

  double eval() const override;
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

  double eval() const override;
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

  double eval() const override;
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

  double eval() const override;
};
