#include "parser.hpp"

#include "../ast/ast.hpp"
#include "../ast/expr.hpp"
#include "../memory/memory.hpp"

Node::Expr *Parser::parse(std::vector<Lexer::Token> tks,
                          Allocator::ArenaAllocator &arena) {
  PStruct p = PStruct{tks, {}, arena, 0};

  while (p.had_tokens()) {
    p.pr.push_back(parse_expr(&p, BindingPower::default_value));
    if (p.current().kind == Lexer::Kind::eof)
      break;
  }

  return p.arena.emplace<ProgramExpr>(p.pr, p.arena);
}

Parser::BindingPower Parser::get_bp(Lexer::Kind kind) {
  switch (kind) {
  case Lexer::Kind::plus:
  case Lexer::Kind::minus:
    return BindingPower::additive;
  case Lexer::Kind::star:
  case Lexer::Kind::slash:
  case Lexer::Kind::mod:
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
  case Lexer::Kind::mod:
    return binary(psr, left, bp);
  default:
    psr->advance();
    return left;
  }
}
