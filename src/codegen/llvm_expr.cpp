#include <llvm/IR/Module.h>

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
String::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                 std::map<std::string, llvm::Value *> &namedValues) const {
  (void)namedValues; 
  (void)ctx;  
  llvm::Value *str = builder.CreateGlobalStringPtr(value);
  return str;
}

llvm::Value *
Binary::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                std::map<std::string, llvm::Value *> &namedValues) const {
  llvm::Value *l = left->codegen(ctx, builder, namedValues);
  llvm::Value *r = right->codegen(ctx, builder, namedValues);

  if (!l || !r)
    return nullptr;

  if (op == "+")
    return builder.CreateAdd(l, r, "addtmp");
  else if (op == "-")
    return builder.CreateSub(l, r, "subtmp");
  else if (op == "*")
    return builder.CreateMul(l, r, "multmp");
  else if (op == "/")
    return builder.CreateSDiv(l, r, "divtmp");
  else {
    std::cerr << "Unknown binary operator: " << op << std::endl;
    return nullptr;
  }
}

llvm::Value *
Call::codegen(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
              std::map<std::string, llvm::Value *> &named_values) const {
  // 1. Resolve the function name
  auto *name_expr = dynamic_cast<Ident *>(name);
  if (!name_expr) {
    std::cerr << "Call target is not a function name\n";
    return nullptr;
  }

  const std::string &func_name = name_expr->ident;
  llvm::Function *callee_func =
      builder.GetInsertBlock()->getModule()->getFunction(func_name);
  if (!callee_func) {
    std::cerr << "Unknown function referenced: " << func_name << "\n";
    return nullptr;
  }

  // 2. Check argument count
  if (callee_func->arg_size() != args.size()) {
    std::cerr << "Incorrect number of arguments passed to function "
              << func_name << "\n";
    return nullptr;
  }

  // 3. Generate code for each argument
  std::vector<llvm::Value *> arg_values;
  for (auto *arg_expr : args) {
    llvm::Value *arg_val = arg_expr->codegen(context, builder, named_values);
    if (!arg_val)
      return nullptr;
    arg_values.push_back(arg_val);
  }

  // 4. Emit call instruction
  return builder.CreateCall(callee_func, arg_values, "calltmp");
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
