#include "../ast/expr.hpp"

llvm::Value *
Number::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                std::map<std::string, llvm::Value *> &namedValues) const {
  (void)namedValues; // unused
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), std::stoll(value));
}

llvm::Value *
Ident::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
               std::map<std::string, llvm::Value *> &namedValues) const {
  auto it = namedValues.find(ident);
  if (it == namedValues.end()) {
    std::cerr << "Unknown variable: " << ident << std::endl;
    return nullptr;
  }
  return builder.CreateLoad(llvm::Type::getInt64Ty(ctx), it->second,
                            ident.c_str());
}

llvm::Value *
Binary::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                std::map<std::string, llvm::Value *> &namedValues) const {
  llvm::Value *l = left->codegen(ctx, builder, namedValues);
  llvm::Value *r = right->codegen(ctx, builder, namedValues);

  if (!l || !r)
    return nullptr;

  std::map<std::string, std::string> op_map = {
      {"+", "addtmp"}, {"-", "subtmp"}, {"*", "multmp"}, {"/", "divtmp"},
      {"%", "modtmp"}, {"==", "eqtmp"}, {"!=", "netmp"}, {"<", "lttmp"},
      {">", "gttmp"},  {"<=", "letmp"}, {">=", "getmp"}};

  auto it = op_map.find(op);
  if (it != op_map.end()) {
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
      return builder.CreateBinOp(llvm::Instruction::BinaryOps::Add, l, r,
                                 it->second.c_str());
    } else if (op == "==" || op == "!=" || op == "<" || op == ">" ||
               op == "<=" || op == ">=") {
      return builder.CreateICmpEQ(l, r, it->second.c_str());
    }
  }
  std::cerr << "Unknown binary operator: " << op << std::endl;
  return nullptr;
}

llvm::Value *
Unary::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
               std::map<std::string, llvm::Value *> &namedValues) const {
  llvm::Value *val = right->codegen(ctx, builder, namedValues);
  if (!val)
    return nullptr;

  if (op == "-")
    return builder.CreateNeg(val, "negtmp");
  else if (op == "+")
    return val; // no-op
  else {
    std::cerr << "Unknown unary operator: " << op << std::endl;
    return nullptr;
  }
}

llvm::Value *
Group::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
               std::map<std::string, llvm::Value *> &namedValues) const {
  return expr->codegen(ctx, builder, namedValues);
}
