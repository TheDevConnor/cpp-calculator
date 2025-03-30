#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/expr.hpp"

Parser::PStruct *Parser::init_parser(std::vector<Lexer::Token> tks,
                                     Allocator::AreanAllocator &a) {
  return new PStruct{a, std::move(tks), 0};
}

Node::Expr *Parser::parse(std::vector<Lexer::Token> tks,
                          Allocator::AreanAllocator &a) {
  PStruct *p = init_parser(std::move(tks), a);

  std::vector<Node::Expr *> pr;
  while (p->had_tokens()) {
    pr.push_back(parse_expr(p, BindingPower::default_value));
    if (p->current().kind == Lexer::Kind::eof)
      break;
  }

  return p->arena.emplace<ProgramExpr>(pr);
}

Parser::BindingPower Parser::get_bp(Lexer::Kind kind) {
  switch (kind) {
  case Lexer::Kind::plus:
  case Lexer::Kind::minus:
    return BindingPower::additive;
  case Lexer::Kind::star:
  case Lexer::Kind::slash:
    return BindingPower::multiplicative;
  case Lexer::Kind::l_paren:
    return BindingPower::group;
  default:
    return BindingPower::default_value;
  }
}

Node::Expr *Parser::nud(PStruct *psr) {
  switch (psr->current().kind) {
  case Lexer::Kind::number:
    return primary(psr);
  case Lexer::Kind::minus:
    return unary(psr);
  case Lexer::Kind::l_paren:
    return grouping(psr);
  default:
    psr->advance();
    return nullptr;
  }
}

Node::Expr *Parser::led(PStruct *psr, Node::Expr *left, BindingPower bp) {
  switch (psr->current().kind) {
  case Lexer::Kind::plus:
  case Lexer::Kind::minus:
  case Lexer::Kind::star:
  case Lexer::Kind::slash:
    return binary(psr, left, bp);
  default:
    psr->advance();
    return left;
  }
}
