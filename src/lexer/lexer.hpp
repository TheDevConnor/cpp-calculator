#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace Lexer {
enum Kind {
  number,
  ident,
  string,
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
  comma,
  equal_equal,
  less_equal,
  less,
  not_equal,
  greater_equal,
  greater,
  l_bracket, // [
  r_bracket, // ]
  l_brace,   // {
  r_brace,   // }
  increment,
  decrement,

  walrus,

  // Keywords
  _uint,
  _int,
  _float,
  _char,
  _bool,
  _str,
  var,
  _const,
  _return,
  fn,
  _module,
  _use,
  print,
  println,
  _alloc,
  _free,
  memcpy,
  _sizeof,
  cast,
  _if,
  _elif,
  _else,
  _struct,
  _enum,
  pub,
  priv,
  loop,

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

  std::unordered_map<std::string, Kind> builtins = {
      {"@module", Kind::_module}, {"@use", Kind::_use},
      {"@output", Kind::print},   {"@outputln", println},
      {"@alloc", Kind::_alloc},   {"@free", Kind::_free},
      {"@memcpy", Kind::memcpy},  {"@sizeof", Kind::_sizeof},
      {"@cast", Kind::cast},
  };

  std::unordered_map<std::string, Kind> keywords = {
      {"uint", Kind::_uint},     {"int", Kind::_int},
      {"float", Kind::_float},   {"char", Kind::_char},
      {"bool", Kind::_bool},     {"str", Kind::_str},
      {"have", Kind::var},       {"const", Kind::_const},
      {"fn", Kind::fn},          {"return", Kind::_return},
      {"if", Kind::_if},         {"else", Kind::_else},
      {"struct", Kind::_struct}, {"enum", Kind::_enum},
      {"pub", Kind::pub},        {"priv", Kind::priv},
      {"loop", Kind::loop},
  };

  static constexpr std::pair<char, Kind> token_map[] = {
      {'+', Kind::plus},      {'-', Kind::minus},     {'*', Kind::star},
      {'/', Kind::slash},     {'%', Kind::mod},       {'(', Kind::l_paren},
      {')', Kind::r_paren},   {';', Kind::semicolon}, {',', Kind::comma},
      {'{', Kind::l_brace},   {'}', Kind::r_brace},   {'[', Kind::l_bracket},
      {']', Kind::r_bracket}, {':', Kind::colon},     {'=', Kind::equals},
      {'<', Kind::less},      {'>', Kind::greater},
  };

  static constexpr std::pair<std::string_view, Kind> double_token_map[] = {
      {"==", Kind::equal_equal}, {"!=", Kind::not_equal},
      {"<=", Kind::less_equal},  {">=", Kind::greater_equal},
      {":=", Kind::walrus}, {"++", Kind::increment},
      {"--", Kind::decrement},
  };

  char advance();
  bool is_at_end();
  char peek(int n);

  Token make_token(Kind k, int whitespace_count);
  Token identifier(int whitespace_count);
  Token number(int whitespace_count);

  Kind check_map(std::string ident);
  int skip_whitespace();

  constexpr std::optional<Kind> lookup_kind(char c) {
    for (auto [key, value] : token_map) {
      if (key == c)
        return value;
    }
    return std::nullopt;
  }

  constexpr std::optional<Kind> lookup_kind(char a, char b) {
    for (auto [key, value] : double_token_map) {
      if (key[0] == a && key[1] == b)
        return value;
    }
    return std::nullopt;
  }
};
}; // namespace Lexer
