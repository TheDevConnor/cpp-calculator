#pragma once

#include <iostream>
#include <string>

#include "ast.hpp"

class SymbolType : public Node::Type {
 public:
  std::string name;

  SymbolType(std::string name) : name(name) { kind = NodeKind::symbol_type; }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "TYPE: " << name;
  }
};
