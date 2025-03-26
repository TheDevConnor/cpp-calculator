#include <iostream>

#include "lexer.hpp"

using namespace Lexer;

char Lexer::lexer::advance() {
  current++;
  start++;
  return current[-1];
}

bool Lexer::lexer::is_at_end() { return *current == '\0'; }
char Lexer::lexer::peek(int n) { return current[n]; }

Token Lexer::lexer::make_token(Kind k) {
  Token tk;

  tk.kind = k;
  tk.start = start;
  tk.value = std::string(current - 1, start);

  return tk;
}

Token Lexer::lexer::number() {
  while (std::isdigit(peek(0)))
    advance();

  if (peek(0) == '.') {
    advance();
    while (std::isdigit(peek(0)))
      advance();
  }

  return make_token(Kind::number);
}

void Lexer::lexer::skip_whitespace() {
  for (;;) {
    char c = peek(0);
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    default:
      return;
    }
  }
}

void Lexer::lexer::init_lexer(Lexer::lexer *lx, const char *source) {
  lx->current = source;
  lx->start = source;
}

Token Lexer::lexer::scan_token() {
  skip_whitespace();
  start = current;

  if (is_at_end())
    return make_token(eof);

  char c = advance();

  if (std::isdigit(c))
    return number();

  switch (c) {
  case '+':
    return make_token(Kind::plus);
  case '-':
    return make_token(Kind::minus);
  case '*':
    return make_token(Kind::star);
  case '/':
    return make_token(Kind::slash);
  case '(':
    return make_token(Kind::l_paren);
  case ')':
    return make_token(Kind::r_paren);
  }

  std::cerr << "Token not found '" << c << "'" << std::endl;
  return make_token(Kind::unknown);
}
