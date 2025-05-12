#include "../ast/stmt.hpp"

llvm::Value *
ProgramStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                     llvm::Module &module,
                     std::map<std::string, llvm::Value *> &namedValues) const {
  for (std::size_t i = 0; i < size; ++i) {
    if (!stmts[i]->codegen(ctx, builder, module, namedValues))
      return nullptr;
  }
  return llvm::Constant::getNullValue(
      llvm::Type::getInt64Ty(ctx)); // dummy return
}

llvm::Value *
FnStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                llvm::Module &module,
                std::map<std::string, llvm::Value *> &namedValues) const {
  std::vector<llvm::Type *> argTypes(size, llvm::Type::getInt64Ty(ctx));
  llvm::FunctionType *funcType =
      llvm::FunctionType::get(llvm::Type::getInt64Ty(ctx), argTypes, false);

  llvm::Function *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, name, module);

  unsigned idx = 0;
  for (auto &arg : function->args()) {
    arg.setName(args[idx]);
    ++idx;
  }

  llvm::BasicBlock *blockBB = llvm::BasicBlock::Create(ctx, "entry", function);
  builder.SetInsertPoint(blockBB);

  namedValues.clear();
  idx = 0;
  for (auto &arg : function->args()) {
    llvm::AllocaInst *alloca =
        builder.CreateAlloca(llvm::Type::getInt64Ty(ctx), 0, arg.getName());
    builder.CreateStore(&arg, alloca);
    namedValues[arg.getName().str()] = alloca;
    ++idx;
  }

  if (!block->codegen(ctx, builder, module, namedValues))
    return nullptr;

  if (builder.GetInsertBlock()->getTerminator() == nullptr)
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), 0));

  return function;
}

llvm::Value *
BlockStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                   llvm::Module &module,
                   std::map<std::string, llvm::Value *> &namedValues) const {
  for (std::size_t i = 0; i < size; ++i) {
    if (!stmt[i]->codegen(ctx, builder, module, namedValues))
      return nullptr;
  }
  return llvm::Constant::getNullValue(
      llvm::Type::getInt64Ty(ctx)); // dummy return
}

llvm::Value *
ExprStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                  llvm::Module &module,
                  std::map<std::string, llvm::Value *> &namedValues) const {
  return expr->codegen(ctx, builder, namedValues); // discard result
}

llvm::Value *
VarStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                 llvm::Module &module,
                 std::map<std::string, llvm::Value *> &namedValues) const {
  llvm::Value *initVal = expr->codegen(ctx, builder, namedValues);
  if (!initVal)
    return nullptr;

  llvm::AllocaInst *alloca =
      builder.CreateAlloca(llvm::Type::getInt64Ty(ctx), 0, name);
  builder.CreateStore(initVal, alloca);
  namedValues[name] = alloca;

  return alloca;
}

llvm::Value *
ReturnStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                    llvm::Module &module,
                    std::map<std::string, llvm::Value *> &namedValues) const {
  if (!expr) {
    builder.CreateRetVoid();
    return nullptr;
  }

  llvm::Value *retVal = expr->codegen(ctx, builder, namedValues);
  if (!retVal)
    return nullptr;

  builder.CreateRet(retVal);
  return retVal;
}
