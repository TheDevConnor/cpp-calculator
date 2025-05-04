#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "error/error.hpp"
#include "lexer/lexer.hpp"
#include "memory/memory.hpp"
#include "parser/parser.hpp"

using namespace Allocator;

std::string read_file(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " build <filename>\n";
    return nullptr;
  }

  const char *filename;
  std::stringstream buffer;

  if (std::strcmp(argv[1], "build") == 0) {
    filename = argv[2];
    std::ifstream file(filename);
    if (!file) {
      std::cerr << "Failed to open file: " << filename << "\n";
      return nullptr;
    }
    buffer << file.rdbuf();
  } else {
    std::cerr
        << "Argument was not one of the following 'build, help, or version'\n";
    return "";
  }

  return buffer.str();
}

// NOTE: Maybe store the filename on the Token Struct
int main(int argc, char *argv[]) {
  ArenaAllocator arena(1024);
  std::string input = read_file(argc, argv);
  if (input == "")
    return -1;  // Argument issue

  Lexer::lexer lx;
  lx.init_lexer(&lx, input.c_str());

  std::vector<Lexer::Token> tks;
  while (true) {
    Lexer::Token tk = lx.scan_token();
    tks.push_back(tk);
    if (tk.kind == Lexer::Kind::eof)
      break;
  }

  if (Error::report_error())
    return 1;  // Lexical Error

  Node::Stmt *program = Parser::parse(tks, arena);

  if (Error::report_error())
    return 2;  // Parser Error

  // TODO: Handle typechecking then codegenerateion
  return 0;
}
