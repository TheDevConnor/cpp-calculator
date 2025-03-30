#pragma once

#include "ast.hpp"
#include <iostream>
#include <vector>

struct ProgramExpr : public Node::Expr {
public:
  std::vector<Node::Expr *> program;

  ProgramExpr(std::vector<Node::Expr *> program) : program(program) {
    kind = NodeKind::program;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    for (Node::Expr *res : program) {
      res->debug();
    }
  }

  ~ProgramExpr() {
    for (Node::Expr *s : program) {
      delete s;
    }
  }
};

struct Number : public Node::Expr {
public:
  std::string value;

  Number(std::string value) : value(value) { kind = NodeKind::number; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "Number Node: " << value << std::endl;
  }
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

  ~Binary() {
    delete left;
    delete right;
  }
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

  ~Unary() { delete right; }
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

  ~Group() { delete expr; }
};
