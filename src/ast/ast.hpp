#pragma once

enum NodeKind { program, number, binary, unary, group };

class Node {
public:
  struct Expr {
    NodeKind kind;
    virtual void debug(int indent = 0) const = 0;
    virtual double eval() const = 0;
    virtual ~Expr() = default;
  };
};

inline Node node;
