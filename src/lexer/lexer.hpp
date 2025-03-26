#pragma once

#include <string>

namespace Lexer {
enum Kind { number, plus, minus, star, slash, l_paren, r_paren, eof, unknown };

struct Token {
  Kind kind;
  std::string value;
  const char *start;
};

class lexer {
public:
  void init_lexer(lexer *lx, const char *source);
  Token scan_token();

private:
  const char *current;
  const char *start;

  char advance();
  bool is_at_end();
  char peek(int n);

  Token make_token(Kind k);
  Token number();

  void skip_whitespace();
};
}; // namespace Lexer
