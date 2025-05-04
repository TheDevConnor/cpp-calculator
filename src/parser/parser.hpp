#pragma once

#include <stdexcept>
#include <vector>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "../memory/memory.hpp"

namespace Parser {
enum BindingPower {
  default_value = 0,
  additive = 1,       // + -
  multiplicative = 2, // * /
  exponent = 3,       // ^
  group = 4,          // ()
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
  Lexer::Token peek(size_t offset = 0) { return tks[pos + offset]; }
  Lexer::Token current() { return tks[pos]; }
  Lexer::Token advance() { return tks[pos++]; }
  Lexer::Token expect(Lexer::Kind tk, std::string msg) {
    if (peek(0).kind == tk)
      return advance();
    throw std::invalid_argument(msg);
    return advance();
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

// type functions
Node::Type *tnud(PStruct *psr);
Node::Type *tled(PStruct *psr, Node::Type *left, BindingPower bp);
BindingPower tget_bp(PStruct *psr, Lexer::Kind tk);

// stmt functions
Node::Stmt *expr_stmt(PStruct *psr);
Node::Stmt *var_stmt(PStruct *psr);
}; // namespace Parser
