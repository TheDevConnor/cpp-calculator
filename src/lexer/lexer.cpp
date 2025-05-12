#include "lexer.hpp"

#include <cctype>
#include <string>

#include "../error/error.hpp"

using namespace Lexer;

const char *Lexer::lexer::line_start(int line) {
  const char *start = source;
  int cline = 1;

  while (cline != line) {
    if (*start == '\n')
      cline++;
    start++;
  }

  return start;
}

char Lexer::lexer::advance() {
  char c = *current++;
  if (c == '\n') {
    line++;
    pos = 0;
  } else {
    pos++;
  }
  return c;
}

bool Lexer::lexer::is_at_end() { return *current == '\0'; }
char Lexer::lexer::peek(int n) { return current[n]; }

Token Lexer::lexer::make_token(Kind k, int whitespace_count) {
  Token tk;

  tk.kind = k;
  tk.start = start;
  tk.value = std::string(start, current);
  tk.whitespace = whitespace_count;
  tk.line = line;
  tk.pos = pos - 1;

  return tk;
}

Lexer::Kind Lexer::lexer::check_map(std::string ident) {
  auto it = keywords.find(ident);
  if (it != keywords.end())
    return it->second;
  return Kind::ident;
}

Token Lexer::lexer::number(int whitespace_count) {
  while (isdigit(peek(0)))
    advance();

  if (peek(0) == '.') {
    advance();
    while (isdigit(peek(0)))
      advance();
  }

  return make_token(Kind::number, whitespace_count);
}

Token Lexer::lexer::identifier(int whitespace_count) {
  while (isalpha(peek(0)) || isdigit(peek(0)) || peek(0) == '_')
    advance();
  std::string ident(start, current);
  return make_token(check_map(ident), whitespace_count);
}

int Lexer::lexer::skip_whitespace() {
  int count = 0;
  for (;;) {
    char c = peek(0);
    switch (c) {
    case '\n':
      advance();
      break;
    case ' ':
    case '\r':
    case '\t':
      advance();
      count++;
      break;
    default:
      return count;
    }
  }
}

void Lexer::lexer::init_lexer(Lexer::lexer *lx, const char *source) {
  lx->current = source;
  lx->start = source;
  lx->source = source;
}

Token Lexer::lexer::scan_token() {
  int whitespace_count = skip_whitespace();
  start = current;

  if (is_at_end())
    return make_token(eof, whitespace_count);

  char c = advance();

  if (isdigit(c))
    return number(whitespace_count);

  if (isalpha(c))
    return identifier(whitespace_count);

  char next = peek(0);
  if (auto kind2 = lookup_kind(c, next)) {
    advance();
    return make_token(*kind2, whitespace_count);
  }
  if (auto kind = lookup_kind(c)) {
    return make_token(*kind, whitespace_count);
  }

  std::string msg = "Token not found '" + std::to_string(c) + "'";
  Error::handle_lexer_error(*this, "Lexical", "math.xi", msg);
  return make_token(Kind::unknown, whitespace_count);
}
