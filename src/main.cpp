#include <cstdlib>
#include <iostream>
#include <vector>

#include "lexer/lexer.hpp"
#include "memory/memory.hpp"
#include "parser/parser.hpp"

using namespace Allocator;

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  ArenaAllocator arena(16);
  const char *input = "5 * 3 * (3 + 5) / 5";

  Lexer::lexer lx;
  lx.init_lexer(&lx, input);

  std::vector<Lexer::Token> tks;
  while (true) {
    Lexer::Token tk = lx.scan_token();
    tks.push_back(tk);
    if (tk.kind == Lexer::Kind::eof)
      break;
  }

  Node::Expr *program = Parser::parse(tks, arena);
  std::cout << program->eval() << std::endl;

  return 0;
}
