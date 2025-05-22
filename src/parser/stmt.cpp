#include "../ast/stmt.hpp"

#include <vector>

#include "parser.hpp"

Node::Stmt *Parser::parse_stmt(PStruct *psr) {
  switch (psr->current().kind) {
  case Lexer::Kind::_module:
    return module_stmt(psr);
  case Lexer::Kind::var:
    return var_stmt(psr);
  case Lexer::Kind::print:
  case Lexer::Kind::println:
    return print_stmt(psr);
  case Lexer::Kind::_const:
    return const_stmt(psr);
  case Lexer::Kind::_return:
    return return_stmt(psr);
  case Lexer::Kind::l_brace:
    return block_stmt(psr);
  case Lexer::Kind::loop:
    return loop_stmt(psr);
  default:
    return expr_stmt(psr);
  }
}

Node::Stmt *Parser::module_stmt(PStruct *psr) {
  psr->expect(Lexer::Kind::_module,
              "Expected the @module keyword to define the module");
  std::string name =
      psr->expect(Lexer::Kind::ident, "Expected a name for the module").value;
  psr->expect(Lexer::Kind::semicolon,
              "Expected ';' at the end of the module stmt");

  return psr->arena.emplace<ModuleStmt>(name);
}

Node::Stmt *Parser::expr_stmt(PStruct *psr) {
  Node::Expr *expr = parse_expr(psr, BindingPower::default_value);
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

Node::Stmt *Parser::print_stmt(PStruct *psr) {
  bool is_ln = psr->current().kind == Lexer::Kind::println;
  if (is_ln)
    psr->expect(Lexer::Kind::println,
                "Expected the keyword 'println' to start a print stmt");
  else
    psr->expect(Lexer::Kind::print,
                "Expected the keyword 'print' to start a print stmt");
  psr->expect(Lexer::Kind::l_paren,
              "Expected an '(' to start the print stmt args");

  Node::Expr *fd = parse_expr(psr, BindingPower::default_value);
  psr->expect(Lexer::Kind::comma, "Expected a ',' after the file descriptor");

  std::vector<Node::Expr *> args;
  while (psr->current().kind != Lexer::Kind::r_paren) {
    args.push_back(parse_expr(psr, BindingPower::default_value));
    if (psr->current().kind == Lexer::Kind::r_paren)
      break;
    psr->expect(Lexer::Kind::comma,
                "Expected a ',' after an argument in the print stmt");
  }
  psr->expect(Lexer::Kind::r_paren,
              "Expected a ')' to end the print stmt args");
  psr->expect(Lexer::Kind::semicolon,
              "Expected a ';' at the end of the print stmt");

  return psr->arena.emplace<PrintStmt>(fd, is_ln, args, psr->arena);
}

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

/* Possable loop variations
 * loop (i = 0; i < 10) : (i++) {}
 * loop (i = 0; i < 10) {}
 * loop (i < 10) : (i++) {}
 * loop (i < 10) {}
 */

Node::Stmt *Parser::loop_stmt(PStruct *psr) {
  psr->expect(Lexer::Kind::loop, "Expected a 'loop' keyword to start a loop");
  
  bool is_for = false;
  Node::Expr *init = nullptr;
  Node::Expr *condition = nullptr;
  Node::Expr *optional = nullptr;

  psr->expect(Lexer::Kind::l_paren, "Expected a '(' to start a loop");
  if (psr->current().kind == Lexer::Kind::ident && psr->peek(1).kind == Lexer::Kind::equals) {
    is_for = true;
    init = parse_expr(psr, BindingPower::default_value);
    psr->expect(Lexer::Kind::semicolon, "Expected a ';' after the init expr");
    condition = parse_expr(psr, BindingPower::default_value);
  } else {
    condition = parse_expr(psr, BindingPower::default_value);
  }
  psr->expect(Lexer::Kind::r_paren, "Expected a ')' to end the loop condition");

  if (psr->current().kind == Lexer::Kind::colon) {
    psr->expect(Lexer::Kind::colon, "Expected a ':' to start the optional expr");
    psr->expect(Lexer::Kind::l_paren, "Expected a '(' to start the optional expr");
    optional = parse_expr(psr, BindingPower::default_value);
    psr->expect(Lexer::Kind::r_paren, "Expected a ')' to end the optional expr");
  }

  Node::Stmt *block = parse_stmt(psr);

  return psr->arena.emplace<LoopStmt>(is_for, init, condition, optional, block);
}
