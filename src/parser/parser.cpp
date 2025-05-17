#include "parser.hpp"

#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"
#include "../ast/type.hpp"
#include "../memory/memory.hpp"

Node::Stmt *Parser::parse(std::vector<Lexer::Token> tks,
                          Allocator::ArenaAllocator &arena) {
  PStruct p = PStruct{tks, {}, arena, 0};

  while (p.had_tokens()) {
    p.pr.push_back(parse_stmt(&p));
    if (p.current().kind == Lexer::Kind::eof)
      break;
  }

  return p.arena.emplace<ProgramStmt>(p.pr, p.arena);
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
    return BindingPower::call;
  default:
    return BindingPower::default_value;
  }
}

Node::Expr *Parser::nud(PStruct *psr) {
  switch (psr->current().kind) {
  case Lexer::Kind::number:
  case Lexer::Kind::ident:
  case Lexer::Kind::string:
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
  case Lexer::Kind::l_paren:
    return _call(psr, left, bp);
  default:
    psr->advance();
    return left;
  }
}

// TODO: Flesh this out later
Node::Type *Parser::parse_type(PStruct *psr) {
  switch (psr->current().kind) {
  case Lexer::Kind::_uint:
  case Lexer::Kind::_int:
  case Lexer::Kind::_float:
  case Lexer::Kind::_bool:
  case Lexer::Kind::_char:
  case Lexer::Kind::_str:
    return psr->arena.emplace<SymbolType>(psr->advance().value);
  default:
    psr->advance();
    return nullptr;
  }
}
