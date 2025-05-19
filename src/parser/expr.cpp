#include "../ast/expr.hpp"

#include "../ast/ast.hpp"
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
  case Lexer::Kind::ident:
    return psr->arena.emplace<Ident>(psr->advance().value);
  case Lexer::Kind::string:
    return psr->arena.emplace<String>(psr->advance().value);
  default:
    std::cerr << "Could not parse primary expr '" << psr->current().value << "'"
              << std::endl;
    return nullptr;
  }
}

Node::Expr *Parser::unary(PStruct *psr) {
  Lexer::Token op = psr->advance();
  Node::Expr *right = parse_expr(psr, BindingPower::default_value);
  // return new Unary(right, op.value);
  return psr->arena.emplace<Unary>(right, op.value);
}

Node::Expr *Parser::grouping(PStruct *psr) {
  psr->advance(); // consume the (
  Node::Expr *expr = parse_expr(psr, BindingPower::default_value);
  psr->advance();
  return psr->arena.emplace<Group>(expr);
}

Node::Expr *Parser::binary(PStruct *psr, Node::Expr *left, BindingPower bp) {
  Lexer::Token op = psr->advance();
  Node::Expr *right = parse_expr(psr, bp);
  // return new Binary(left, right, op.value);
  return psr->arena.emplace<Binary>(left, right, op.value);
}

Node::Expr *Parser::_call(PStruct *psr, Node::Expr *left, BindingPower bp) {
  psr->advance(); // consume the (
  std::vector<Node::Expr *> args;
  while (psr->current().kind != Lexer::Kind::r_paren) {
    args.push_back(parse_expr(psr, BindingPower::default_value));
    if (psr->current().kind == Lexer::Kind::comma)
      psr->advance();
  }
  psr->advance(); // consume the )
  return psr->arena.emplace<Call>(left, args);
}

Node::Expr *Parser::assign(PStruct *psr, Node::Expr *left, BindingPower bp) {
  (void)bp;

  Lexer::Token op = psr->current();
  Node::Expr *right = parse_expr(psr, BindingPower::default_value);

  return psr->arena.emplace<Assign>(op, left, right);
}
