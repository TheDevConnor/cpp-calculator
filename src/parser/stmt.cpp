#include "../ast/stmt.hpp"

#include <vector>

#include "parser.hpp"

Node::Stmt *Parser::parse_stmt(PStruct *psr) {
  switch (psr->current().kind) {
  case Lexer::Kind::var:
    return var_stmt(psr);
  case Lexer::Kind::_const:
    return const_stmt(psr);
  case Lexer::Kind::_return:
    return return_stmt(psr);
  case Lexer::Kind::l_brace:
    return block_stmt(psr);
  default:
    return expr_stmt(psr);
  }
}

Node::Stmt *Parser::expr_stmt(PStruct *psr) {
  Node::Expr *expr = parse_expr(psr, BindingPower::default_value);
  if (psr->current().kind == Lexer::Kind::semicolon) {
    psr->advance();
  } else {
    Error::handle_error("Parser", "main.xi",
                        "Expected a ';' at the end of an expr_stmt", psr->tks,
                        psr->current().line, psr->current().pos);
  }
  return psr->arena.emplace<ExprStmt>(expr);
}

Node::Stmt *Parser::const_stmt(PStruct *psr) {
  psr->expect(Lexer::Kind::_const,
              "Expected the keyword 'const' to start a const stmt");
  std::string name =
      psr->expect(Lexer::Kind::ident,
                  "Expected an 'ident' for the name of a const stmt")
          .value;
  psr->expect(Lexer::Kind::walrus,
              "Expected a ':=' after the name to declare the body");

  switch (psr->current().kind) {
  case Lexer::Kind::fn:
    return fn_stmt(psr, name);
  default:
    std::string msg = "Expected a 'const' stmt to lead to either an enum, "
                      "struct, or function";
    Error::handle_error("Parser", "main.xi", msg, psr->tks, psr->current().line,
                        psr->current().pos);
    break;
  }

  return nullptr;
};

Node::Stmt *Parser::fn_stmt(PStruct *psr, std::string name) {
  psr->expect(Lexer::Kind::fn,
              "Expected 'fn' keyword to start a function delcaration");
  psr->expect(Lexer::Kind::l_paren, "Expected an '(' to define args");

  std::vector<std::pair<std::string, Node::Type *>> params;
  while (psr->current().kind != Lexer::Kind::r_paren) {
    std::string pname = psr->expect(Lexer::Kind::ident,
                                    "Expected an identifier for the arg name")
                            .value;
    psr->expect(Lexer::Kind::colon,
                "Expected a ':' before you declare the arg type");
    Node::Type *ptype = parse_type(psr);

    params.push_back({pname, ptype});

    if (psr->current().kind == Lexer::Kind::r_paren)
      break;
    psr->expect(Lexer::Kind::comma,
                "Expected a ',' after a parameter in a function");
  }
  psr->expect(Lexer::Kind::r_paren, "Expected an ')' to close the args");

  // parse the return type
  Node::Type *type = parse_type(psr);
  if (type == nullptr)
    Error::handle_error("Parser", "main.xi",
                        "Expected a return type for the function", psr->tks,
                        psr->current().line, psr->current().pos);

  Node::Stmt *block = parse_stmt(psr);
  psr->expect(Lexer::Kind::semicolon,
              "Expected a ';' at the end of a function declaration");

  return psr->arena.emplace<FnStmt>(name, type, params, block, psr->arena);
}

Node::Stmt *Parser::var_stmt(PStruct *psr) {
  psr->expect(Lexer::Kind::var,
              "Expected the keyword 'have' to start a var declaration");
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

Node::Stmt *Parser::block_stmt(PStruct *psr) {
  std::vector<Node::Stmt *> block;
  psr->expect(Lexer::Kind::l_brace, "Expected a '{' to start a block");

  while (psr->current().kind != Lexer::Kind::r_brace) {
    block.push_back(parse_stmt(psr));
  }
  psr->expect(Lexer::Kind::r_brace, "Expected a '}' to end a block");

  return psr->arena.emplace<BlockStmt>(block, psr->arena);
};

Node::Stmt *Parser::return_stmt(PStruct *psr) {
  psr->expect(Lexer::Kind::_return,
              "Expected a RETURN keyword to start a return stmt");
  if (psr->peek().kind == Lexer::Kind::semicolon) {
    psr->advance();
    return psr->arena.emplace<ReturnStmt>(nullptr);
  }
  Node::Expr *expr = parse_expr(psr, BindingPower::default_value);
  psr->expect(Lexer::Kind::semicolon,
              "Expected a ';' at the end of a return stmt");
  return psr->arena.emplace<ReturnStmt>(expr);
}
