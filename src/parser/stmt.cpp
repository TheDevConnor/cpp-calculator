#include "../ast/stmt.hpp"
#include "parser.hpp"

Node::Stmt *Parser::parse_stmt(PStruct *psr) {
  switch (psr->current().kind) {
  case Lexer::Kind::var:
    return var_stmt(psr);
  default:
    return expr_stmt(psr);
  }
}

Node::Stmt *Parser::expr_stmt(PStruct *psr) {
  Node::Expr *expr = parse_expr(psr, BindingPower::default_value);
  return psr->arena.emplace<ExprStmt>(expr);
}

Node::Stmt *Parser::var_stmt(PStruct *psr) {
  psr->expect(Lexer::Kind::var,
              "Expected the keyword 'own' to start a var declaration");
  std::string name =
      psr->expect(Lexer::Kind::ident, "Expected a name for the var declaration")
          .value;
  psr->expect(Lexer::Kind::colon, "Expected ':' before you define the type");
  Node::Type *type = parse_type(psr);
  psr->expect(Lexer::Kind::equals,
              "Expected an '=' before you define the body");
  Node::Expr *expr = parse_expr(psr, BindingPower::default_value);
  psr->expect(Lexer::Kind::semicolon,
              "Expected ';' at the end of an expr_stmt");

  return psr->arena.emplace<VarStmt>(name, type, expr);
}
