#include <iostream>
#include <vector>

#include "lexer/lexer.hpp"
#include "memory/memory.hpp"
#include "parser/parser.hpp"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  Allocator::AreanAllocator arena(1024);

  const char *input = "1 + (9 / 3) - 4 * 8";
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

  arena.reset();
  return 0;
}
