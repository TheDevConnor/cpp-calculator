#pragma once

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include <vector>

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
  size_t pos;

  bool had_tokens(PStruct *psr) { return psr->pos < psr->tks.size(); }
  Lexer::Token peek(PStruct *psr, int offset = 0) {
    return psr->tks[psr->pos + offset];
  }
  Lexer::Token current(PStruct *psr) { return psr->tks[psr->pos]; }
  Lexer::Token advance(PStruct *psr) { return psr->tks[psr->pos++]; }
};

namespace Parser {
Parser::PStruct *init_parser(std::vector<Lexer::Token> tks);
Node::Expr *parse(std::vector<Lexer::Token> tks);
Node::Expr *parse_expr(PStruct *psr, BindingPower bp);

Node::Expr *nud(PStruct *psr);
Node::Expr *led(PStruct *psr, Node::Expr *left, BindingPower bp);
BindingPower get_bp(Lexer::Kind kind);

// nud functions
Node::Expr *primary(PStruct *psr);
Node::Expr *unary(PStruct *psr);
Node::Expr *grouping(PStruct *psr);

// led functions
Node::Expr *binary(PStruct *psr, Node::Expr *left, BindingPower bp);
}; // namespace Parser
