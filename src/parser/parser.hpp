#pragma once

#include <vector>

#include "../ast/ast.hpp"
#include "../error/error.hpp"
#include "../lexer/lexer.hpp"
#include "../memory/memory.hpp"

namespace Parser {

enum BindingPower {
  default_value = 0,
  comma = 1,
  assignment = 2,
  ternary = 3,
  logicalOr = 4,
  logicalAnd = 5,
  relational = 6,
  comparison = 7,
  additive = 8,
  multiplicative = 9,
  range = 9,
  power = 10,
  prefix = 11,
  postfix = 12,
  call = 13,
  member = 13,
  _primary = 14,
  err = 15
};

struct PStruct;
inline Lexer::lexer lx;
}; // namespace Parser

struct Parser::PStruct {
  std::vector<Lexer::Token> tks;
  std::vector<Node::Stmt *> pr;
  Allocator::ArenaAllocator &arena;
  size_t pos;

  bool had_tokens() { return pos < tks.size(); }
  Lexer::Token peek(size_t offset = 0) {
    if (pos + offset >= tks.size())
      return tks.back();
    return tks[pos + offset];
  }
  Lexer::Token current() { return (pos >= tks.size()) ? tks.back() : tks[pos]; }
  Lexer::Token advance() {
    return (pos >= tks.size()) ? tks.back() : tks[pos++];
  }
  Lexer::Token expect(Lexer::Kind tk, std::string msg) {
    if (peek(0).kind == tk)
      return advance();
    Error::handle_error("Parser", "main.xi", msg, tks, current().line,
                        current().pos);
    return current();
  }
};

namespace Parser {
Node::Stmt *parse(std::vector<Lexer::Token> tks,
                  Allocator::ArenaAllocator &arena);
Node::Expr *parse_expr(PStruct *psr, BindingPower bp);
Node::Stmt *parse_stmt(PStruct *psr);
Node::Type *parse_type(PStruct *psr);

Node::Expr *nud(PStruct *psr);
Node::Expr *led(PStruct *psr, Node::Expr *left, BindingPower bp);
BindingPower get_bp(Lexer::Kind kind);

// nud functions
Node::Expr *primary(PStruct *psr);
Node::Expr *unary(PStruct *psr);
Node::Expr *grouping(PStruct *psr);

// led functions
Node::Expr *binary(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *_call(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *assign(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *_prefix(PStruct *psr, Node::Expr *left, BindingPower bp);

// type functions
Node::Type *tnud(PStruct *psr);
Node::Type *tled(PStruct *psr, Node::Type *left, BindingPower bp);
BindingPower tget_bp(PStruct *psr, Lexer::Kind tk);

// stmt functions
Node::Stmt *module_stmt(PStruct *psr);
Node::Stmt *expr_stmt(PStruct *psr);
Node::Stmt *var_stmt(PStruct *psr);
Node::Stmt *const_stmt(PStruct *psr);
Node::Stmt *print_stmt(PStruct *psr);
Node::Stmt *fn_stmt(PStruct *psr, std::string name);
Node::Stmt *block_stmt(PStruct *psr);
Node::Stmt *return_stmt(PStruct *psr);
Node::Stmt *loop_stmt(PStruct *psr);
}; // namespace Parser
