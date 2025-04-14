#include <cmath>

#include "../ast/expr.hpp"

double ProgramExpr::eval() const {
  double finale = 0;
  for (Node::Expr *res : program) finale = res->eval();
  return finale;
}

double Number::eval() const {
  return std::stod(value);
}

double Binary::eval() const {
  double lhs = left->eval();
  double rhs = right->eval();

  switch (op[0]) {
    case '+':
      return lhs + rhs;
    case '-':
      return lhs - rhs;
    case '*':
      return lhs * rhs;
    case '/':
      return lhs / rhs;
    case '%':
      return std::fmod(lhs, rhs);
    default:
      std::cerr << "Unknown operator '" + op + "'" << std::endl;
      exit(-1);
  }
}

double Unary::eval() const {
  double rhs = right->eval();
  switch (op[0]) {
    case '-':
      return -rhs;
    default:
      std::cerr << "Unknown operator '" + op + "'" << std::endl;
      exit(-1);
  }
}

double Group::eval() const {
  return expr->eval();
}
