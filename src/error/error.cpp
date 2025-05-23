#include "error.hpp"

#include <iostream>
#include <string>

#include "../color/color.hpp"

Color col;

std::string Error::error_head(std::string error_type, int line, int pos,
                              std::string filepath) {
  std::string ln = col.color(std::to_string(line), Color::YELLOW, true, false);
  std::string ps = col.color(std::to_string(pos), Color::YELLOW, true, false);
  std::string error = col.color("error", Color::RED, false, true) + ": ";
  std::string type = col.color(error_type, Color::WHITE, true, true);
  return error + type + "\n  --> [" + ln + "::" + ps + "](" + filepath + ")\n";
}

std::string Error::generate_whitespace(int space) {
  std::string final;
  for (int i = 0; i < space; i++)
    final += " ";
  return final;
}

std::string Error::generate_line(const std::vector<Lexer::Token> &tks, int line, int pos) {
  (void)pos;
  std::string ln = "";

  // Safety check to handle empty token list
  if (tks.empty()) {
    return ln + "\n";
  }

  try {
    for (const Lexer::Token &tk : tks) {
      if (tk.line != line) continue;
      ln += col.color(generate_whitespace(tk.whitespace) + tk.value, Color::WHITE, false, true);
    }
    ln += "\n";
    return ln;
  } catch (const std::exception &e) {
    // If any exception occurs while generating the line, return a safe default
    return "Error generating line: " + std::string(e.what()) + "\n";
  }
}

bool Error::report_error() {
  if (errors.size() > 0) {
    std::cout << "Total Errors: "
              << col.color(std::to_string(errors.size()), Color::RED) << "\n";
    for (std::string error : errors)
      std::cout << error << std::endl;
    return true;
  }
  return false;
}

void Error::handle_lexer_error(Lexer::lexer &lex, std::string error_type,
                               std::string file_path, std::string msg) {
  try {
    const char *start = lex.line_start(lex.line);
    const char *end = start;
    while (*end != '\n' && *end != '\0')
      end++;

    std::string error = error_head(error_type, lex.line, lex.pos, file_path);
    error += col.color("   |\n", Color::GRAY);
    std::string formatted_line =
        line_number(lex.line) + std::to_string(lex.line) + "|";
    error +=
        " " + formatted_line + std::string(start, unsigned(end - start)) + "\n";
    std::string error_space = std::string(static_cast<std::size_t>(std::max(0, lex.pos - 1)), ' ');
    error += col.color("   |", Color::GRAY) + error_space + col.color("^", Color::RED, true, true) + "\n";
    error += col.color("note", Color::CYAN) + ": " + msg;

    errors.push_back(error);
  } catch (const std::exception &e) {
    // If error formatting fails, make sure we at least report something
    std::string simpleError = "Error in " + file_path + " at line " +
                              std::to_string(lex.line) + ", pos " +
                              std::to_string(lex.pos) + ": " + msg +
                              " (Error formatting failed: " + e.what() + ")";
    errors.push_back(simpleError);
  }
}

void Error::handle_error(std::string error_type, std::string file_path,
                         std::string msg, const std::vector<Lexer::Token> &tks,
                         int line, int pos) {
  if (msg.find("Expected ';'") == 0) line = line - 1;
  try {
    std::string error = error_head(error_type, line, pos, file_path);
    error += col.color("   |\n", Color::GRAY);
    std::string formatted_line = line_number(line) + std::to_string(line) + "|";

    // Handle the case when we're at the end of the file
    if (tks.empty()) {
      error += " " + formatted_line + "\n";
    } else {
      error += " " + formatted_line + generate_line(tks, line, pos);
    }

    // Make sure we don't generate negative spaces
    int pointer_pos = pos > 0 ? pos - 1 : 0;
    std::string error_space = std::string(static_cast<std::size_t>(std::max(0, pointer_pos)), '~');
    error += col.color("   |", Color::GRAY) + col.color(error_space, Color::RED) + col.color("^", Color::RED, true, true) + "\n";
    error += col.color("note", Color::CYAN) + ": " + msg;

    errors.push_back(error);
  } catch (const std::exception &e) {
    // If any exception occurs while formatting the error, fallback to a simple message
    std::string simpleError = "Error in " + file_path + " at line " +
                              std::to_string(line) + ", pos " +
                              std::to_string(pos) + ": " + msg +
                              " (Error formatting failed: " + e.what() + ")";
    errors.push_back(simpleError);
  }
}
