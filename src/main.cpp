#include <cstring>
#include <fstream>
#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>
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
  llvm::LLVMContext context;
  llvm::IRBuilder<> builder(context);
  llvm::Module module("main", context);
  ArenaAllocator arena(1024);

  std::string input = read_file(argc, argv);
  if (input == "")
    return -1; // Argument issue

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
    return 1; // Lexical Error

  Node::Stmt *program = Parser::parse(tks, arena);
  program->debug();

  if (Error::report_error())
    return 2; // Parser Error

  // NOTE: Handle type checking here

  // Code generation
  std::map<std::string, llvm::Value *> named_values;
  llvm::Value *result = program->codegen(context, builder, module, named_values);
  if (result == nullptr) {
    std::cerr << "Error generating code\n";
    return 3; // Code generation error
  }

  std::error_code EC;
  llvm::raw_fd_ostream outFile("out.ll", EC);
  if (EC) {
    std::cerr << "Could not open out.ll: " << EC.message() << "\n";
    return 4;
  }
  module.print(outFile, nullptr);

  // Now we want to compile and link the IR file programmatically
  // Step 1: Use llc to compile IR to an object file
  if (system("llc -filetype=obj -relocation-model=pic out.ll -o out.o") != 0) {
    std::cerr << "Error running llc to generate object file!" << std::endl;
    return -1;
  }

  // Step 2: Link the object file to an executable using clang
  if (system("clang out.o -o my_program -fPIE") != 0) {
    std::cerr << "Error linking with clang!" << std::endl;
    return -1;
  }

  // Step 3: Execute the generated program
  std::cout << "Executable 'my_program' has been generated!" << std::endl;

  // delete the out.ll and out.o files
  // NOTE: Add in a save option if the user wants to keep the files
  // if (system("rm out.ll out.o") != 0) {
  //   std::cerr << "Error deleting temporary files!" << std::endl;
  //   return -1;
  // }

  return 0;
}
