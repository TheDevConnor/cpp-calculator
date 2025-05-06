#pragma once

enum NodeKind {
  symbol_type,
  program,
  number,
  ident,
  binary,
  unary,
  group,
  expr_stmt,
  var_stmt,
  return_stmt,
  fn_stmt,
  block_stmt,
};

class Node {
 public:
  struct Expr {
    NodeKind kind;
    virtual void debug(int indent = 0) const = 0;
  };

  struct Stmt {
    NodeKind kind;
    virtual void debug(int indent = 0) const = 0;
  };

  struct Type {
    NodeKind kind;
    virtual void debug(int indent = 0) const = 0;
  };
};

inline Node node;
