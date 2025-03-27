#include <vector>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  // NOTE: Grouping to the left segfaults
  const char *input = "1 + 2 * 1 + 0.45 / 2";
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
  program->debug();

  delete program;
  return 0;
}
