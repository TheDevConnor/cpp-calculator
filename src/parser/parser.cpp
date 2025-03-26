#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/expr.hpp"

Parser::PStruct *Parser::init_parser(std::vector<Lexer::Token> tks) {
  return new PStruct{tks, 0};
}

Node::Expr *Parser::parse(std::vector<Lexer::Token> tks) {
  PStruct *p = init_parser(tks);

  std::vector<Node::Expr *> program;
  while (p->had_tokens(p)) {
    program.push_back(parse_expr(p, BindingPower::default_value));
    if (p->current(p).kind == Lexer::Kind::eof)
      break;
  }

  return new ProgramExpr(program);
}

Parser::BindingPower Parser::get_bp(Lexer::Kind kind) {
  switch (kind) {
  case Lexer::Kind::plus:
  case Lexer::Kind::minus:
    return BindingPower::additive;
  case Lexer::Kind::star:
  case Lexer::Kind::slash:
    return BindingPower::multiplicative;
  default:
    return BindingPower::default_value;
  }
}

Node::Expr *Parser::nud(PStruct *psr) {
  switch (psr->current(psr).kind) {
  case Lexer::Kind::number:
    return primary(psr);
  case Lexer::Kind::minus:
    return unary(psr);
  case Lexer::Kind::l_paren:
    return grouping(psr);
  default:
    psr->advance(psr);
    return nullptr;
  }
}

Node::Expr *Parser::led(PStruct *psr, Node::Expr *left, BindingPower bp) {
  switch (psr->current(psr).kind) {
  case Lexer::Kind::plus:
  case Lexer::Kind::minus:
  case Lexer::Kind::star:
  case Lexer::Kind::slash:
    return binary(psr, left, bp);
  default:
    psr->advance(psr);
    return left;
  }
}
