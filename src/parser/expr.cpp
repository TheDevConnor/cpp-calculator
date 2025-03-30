#include "../ast/expr.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"
#include <string>

Node::Expr *Parser::parse_expr(PStruct *psr, BindingPower bp) {
  std::cout << "Current -> " << psr->current(psr).value << std::endl;
  Node::Expr *left = nud(psr);
  std::cout << "Current -> " << psr->current(psr).value << std::endl;

  while (get_bp(psr->current(psr).kind) > bp) {
    left = led(psr, left, get_bp(psr->current(psr).kind));
  }

  return left;
}

Node::Expr *Parser::primary(PStruct *psr) {
  switch (psr->current(psr).kind) {
  case Lexer::Kind::number:
    return new Number(psr->advance(psr).value);
  default:
    std::cerr << "Could not parse primary expr '" << psr->current(psr).value
              << "'" << std::endl;
    return nullptr;
  }
}

Node::Expr *Parser::unary(PStruct *psr) {
  Lexer::Token op = psr->advance(psr);
  Node::Expr *right = parse_expr(psr, BindingPower::default_value);
  return new Unary(right, op.value);
}

Node::Expr *Parser::binary(PStruct *psr, Node::Expr *left, BindingPower bp) {
  Lexer::Token op = psr->advance(psr);
  Node::Expr *right = parse_expr(psr, bp);
  return new Binary(left, right, op.value);
}

Node::Expr *Parser::grouping(PStruct *psr) {
  psr->advance(psr); // consume the (
  Node::Expr *expr = parse_expr(psr, BindingPower::group);
  // We don't need to advance because our lookups do it for us already
  return new Group(expr);
}
