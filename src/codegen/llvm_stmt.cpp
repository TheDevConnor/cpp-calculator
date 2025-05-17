#include "../ast/stmt.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

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
ModuleStmt ::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                     llvm::Module &module,
                     std::map<std::string, llvm::Value *> &namedValues) const {
  (void)ctx;
  (void)builder;
  (void)module;
  (void)namedValues;

  module.setModuleIdentifier(name);

  return llvm::Constant::getNullValue(
      llvm::Type::getInt64Ty(ctx)); // dummy return
}

llvm::Value *
FnStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                llvm::Module &module,
                std::map<std::string, llvm::Value *> &locals) const {
  llvm::Type *ret_type = return_type->codegen(ctx);

  std::vector<llvm::Type *> param_types;
  for (size_t i = 0; i < size; ++i)
    param_types.push_back(args_type[i]->codegen(ctx));

  llvm::FunctionType *fn_type =
      llvm::FunctionType::get(ret_type, param_types, false);

  llvm::Function *fn = llvm::Function::Create(
      fn_type, llvm::Function::ExternalLinkage, name, module);

  // Create new entry block
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(ctx, "entry", fn);
  builder.SetInsertPoint(entry);

  // Set up function arguments
  size_t idx = 0;
  for (auto &arg : fn->args()) {
    arg.setName(args[idx]);

    llvm::AllocaInst *alloca =
        builder.CreateAlloca(arg.getType(), nullptr, args[idx]);
    builder.CreateStore(&arg, alloca);
    locals[args[idx]] = alloca;

    idx++;
  }

  // Emit function body
  block->codegen(ctx, builder, module, locals);

  // If no return statement, emit default return (void or 0)
  if (!builder.GetInsertBlock()->getTerminator()) {
    if (ret_type->isVoidTy())
      builder.CreateRetVoid();
    else
      builder.CreateRet(llvm::Constant::getNullValue(ret_type));
  }

  return fn;
}

llvm::Value *PrintStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                                llvm::Module &module,
                                std::map<std::string, llvm::Value *> &namedValues) const {
  llvm::Value *fd_val = fd->codegen(ctx, builder, namedValues);
  if (!fd_val)
    return nullptr;
  
  // For now, we only handle one argument
  llvm::Value *msg_val = args[0]->codegen(ctx, builder, namedValues);

  llvm::StringRef raw_str;
  if (auto *global = llvm::dyn_cast<llvm::GlobalVariable>(msg_val)) {
    if (auto *data = llvm::dyn_cast<llvm::ConstantDataArray>(global->getInitializer())) {
      raw_str = data->getAsCString();
    }
  }

  std::string str_to_print = raw_str.str();
  if (is_ln && !str_to_print.empty()) {
    str_to_print[str_to_print.size() - 1] = '\n';
  }

  llvm::Value *str_ptr = builder.CreateGlobalStringPtr(str_to_print);
  llvm::Value *len_val = builder.getInt64(str_to_print.size());

  // Declare write once
  llvm::Function *write_fn = module.getFunction("write");
  if (!write_fn) {
    llvm::FunctionType *write_type = llvm::FunctionType::get(
        llvm::Type::getInt64Ty(ctx),
        // fd, str, len
        {llvm::Type::getInt32Ty(ctx), llvm::Type::getInt8Ty(ctx)->getPointerTo(),
         llvm::Type::getInt64Ty(ctx)},
        false);
    // Create the function
    write_fn = llvm::Function::Create(write_type, llvm::Function::ExternalLinkage, "write", module);
  }

  // Cast fd to i32 if needed
  if (fd_val->getType()->getIntegerBitWidth() != 32)
    fd_val = builder.CreateTrunc(fd_val, llvm::Type::getInt32Ty(ctx));

  return builder.CreateCall(write_fn, {fd_val, str_ptr, len_val});
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
  (void) module;
  return expr->codegen(ctx, builder, namedValues); // discard result
}

llvm::Value *
VarStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                 llvm::Module &module,
                 std::map<std::string, llvm::Value *> &namedValues) const {
  (void) module;
  llvm::Value *initVal = expr->codegen(ctx, builder, namedValues);
  if (!initVal)
    return nullptr;

  llvm::AllocaInst *alloca =
      builder.CreateAlloca(type->codegen(ctx), nullptr, name);
  builder.CreateStore(initVal, alloca);
  namedValues[name] = alloca;

  return alloca;
}

llvm::Value *
ReturnStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                    llvm::Module &module,
                    std::map<std::string, llvm::Value *> &namedValues) const {
  (void) module;
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
