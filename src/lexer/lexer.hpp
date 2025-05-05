#pragma once

#include <string>
#include <unordered_map>

namespace Lexer {
enum Kind {
  number,
  ident,
  plus,
  minus,
  star,
  slash,
  mod,
  l_paren,
  r_paren,
  semicolon,
  colon,
  equals,

  // Keywords
  _uint,
  _int,
  _float,
  _char,
  _bool,
  _str,
  var,

  eof,
  unknown,
};

struct Token {
  Kind kind;
  std::string value;
  const char *start;
  int whitespace, line, pos;
};

class lexer {
 public:
  void init_lexer(lexer *lx, const char *source);
  const char *line_start(int line);
  Token scan_token();

  int line = 1;
  int pos = 0;

 private:
  const char *current;
  const char *start;
  const char *source;

  std::unordered_map<std::string, Kind> keywords = {
      {"uint", _uint},
      {"int", _int},
      {"float", _float},
      {"char", _char},
      {"bool", _bool},
      {"str", _str},
      {"have", var},
  };

  char advance();
  bool is_at_end();
  char peek(int n);

  Token make_token(Kind k, int whitespace_count);
  Token identifier(int whitespace_count);
  Token number(int whitespace_count);

  Kind check_map(std::string ident);
  int skip_whitespace();
};
};  // namespace Lexer
