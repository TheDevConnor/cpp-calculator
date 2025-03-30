#pragma once

#include "ast.hpp"
#include <iostream>
#include <string>
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

  double eval() const override {
    double finale = 0;
    for (Node::Expr *res : program) {
      finale = res->eval();
    }
    return finale;
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

  double eval() const override { return std::stod(value); }
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

  double eval() const override {
    double lhs = left->eval();
    double rhs = right->eval();

    switch (op[0]) {
    case '+':
      return lhs + rhs;
    case '-':
      return lhs - rhs;
    case '*':
      return lhs * rhs;
    case '/':
      return lhs / rhs;
    default:
      std::cerr << "Unknown operator '" + op + "'" << std::endl;
      exit(-1);
    }
  };

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

  double eval() const override {
    double rhs = right->eval();
    switch (op[0]) {
    case '-':
      return -rhs;
    default:
      std::cerr << "Unknown operator '" + op + "'" << std::endl;
      exit(-1);
    }
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

  double eval() const override { return expr->eval(); }

  ~Group() { delete expr; }
};
