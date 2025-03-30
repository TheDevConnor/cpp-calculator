#include "../ast/expr.hpp"
#include "../ast/ast.hpp"
#include "../memory/memory.hpp"
#include "parser.hpp"

Node::Expr *Parser::parse_expr(PStruct *psr, BindingPower bp) {
  Node::Expr *left = nud(psr);

  while (get_bp(psr->current().kind) > bp) {
    left = led(psr, left, get_bp(psr->current().kind));
  }

  return left;
}

Node::Expr *Parser::primary(PStruct *psr) {
  switch (psr->current().kind) {
  case Lexer::Kind::number:
    return psr->arena.emplace<Number>(psr->advance().value);
  default:
    std::cerr << "Could not parse primary expr '" << psr->current().value << "'"
              << std::endl;
    return nullptr;
  }
}

Node::Expr *Parser::unary(PStruct *psr) {
  Lexer::Token op = psr->advance();
  Node::Expr *right = parse_expr(psr, BindingPower::default_value);
  return psr->arena.emplace<Unary>(right, op.value);
}

Node::Expr *Parser::binary(PStruct *psr, Node::Expr *left, BindingPower bp) {
  Lexer::Token op = psr->advance();
  Node::Expr *right = parse_expr(psr, bp);
  return psr->arena.emplace<Binary>(left, right, op.value);
}

Node::Expr *Parser::grouping(PStruct *psr) {
  psr->advance(); // consume the (
  Node::Expr *expr = parse_expr(psr, BindingPower::default_value);
  psr->advance();
  return psr->arena.emplace<Group>(expr);
}
