#include <cstdlib>
#include <iostream>
#include <vector>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  const char *input = "5 * 3 * (3 + 5)";

  Lexer::lexer lx;
  lx.init_lexer(&lx, input);

  std::vector<Lexer::Token> tks;
  while (true) {
    Lexer::Token tk = lx.scan_token();
    tks.push_back(tk);
    if (tk.kind == Lexer::Kind::eof)
      break;
  }

  Node::Expr *program = Parser::parse(tks);

  std::cout << program->eval() << std::endl;

  delete program;
  return 0;
}
