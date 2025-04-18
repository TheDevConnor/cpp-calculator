#pragma once

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
  std::vector<Node::Expr *> pr;
  Allocator::ArenaAllocator &arena;
  size_t pos;

  bool had_tokens() { return pos < tks.size(); }
  Lexer::Token peek(size_t offset = 0) { return tks[pos + offset]; }
  Lexer::Token current() { return tks[pos]; }
  Lexer::Token advance() { return tks[pos++]; }
};

namespace Parser {
Node::Expr *parse(std::vector<Lexer::Token> tks,
                  Allocator::ArenaAllocator &arena);
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
