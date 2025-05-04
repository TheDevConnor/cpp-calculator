#pragma once

#include <string>
#include <vector>

#include "../lexer/lexer.hpp"

/*
 * Total errors: 1
 * error: TYPE_OF_ERROR
 *   --> [1::10](main.x)
 *    |
 *  1 | 5 + 7 / 0
 *    |          ^
 * note: ERROR_MSG
 */

class Error {
public:
  inline static std::vector<std::string> errors = {};
  static void handle_lexer_error(Lexer::lexer &lex, std::string error_type,
                                 std::string file_path, std::string msg);
  static void handle_parser_error();
  static bool report_error();

private:
  static std::string error_head(std::string error_type, int line, int pos,
                                std::string filepath);

  static std::string line_number(int line) { return (line < 10) ? "0" : ""; }
};
