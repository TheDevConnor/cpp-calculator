#include "../ast/stmt.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
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

llvm::Value *
EnumStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                  llvm::Module &module,
                  std::map<std::string, llvm::Value *> &namedValues) const {
  (void)ctx;
  (void)builder;
  (void)module;
  (void)namedValues;
  std::vector<llvm::Type *> enum_types;
  for (size_t i = 0; i < size; ++i) {
    enum_types.push_back(llvm::Type::getInt32Ty(ctx));
  }

  llvm::StructType *enum_type = llvm::StructType::create(ctx, enum_types, name);

  std::vector<llvm::Constant *> enum_values;
  for (size_t i = 0; i < size; ++i) {
    enum_values.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), i));
  }

  llvm::Constant *initializer = llvm::ConstantStruct::get(enum_type, enum_values);

  llvm::GlobalVariable *enum_var = new llvm::GlobalVariable(
      module, enum_type, false, llvm::GlobalValue::ExternalLinkage,
      initializer, name);

  enum_var->setAlignment(llvm::Align(4));
  namedValues[name] = enum_var;
  return enum_var;
}

static std::string interpretEscapes(const std::string &s) {
  std::string out;
  for (size_t i = 0; i < s.length(); ++i) {
    if (s[i] == '\\' && i + 1 < s.length()) {
      switch (s[++i]) {
      case 'n':
        out += '\n';
        break;
      case 't':
        out += '\t';
        break;
      case '\\':
        out += '\\';
        break;
      case '"':
        out += '"';
        break;
      default:
        out += s[i];
        break;
      }
    } else {
      out += s[i];
    }
  }
  return out;
}

llvm::Value *
PrintStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                   llvm::Module &module,
                   std::map<std::string, llvm::Value *> &namedValues) const {
  llvm::Value *fd_val = fd->codegen(ctx, builder, namedValues);
  if (!fd_val)
    return nullptr;

  llvm::Function *itoa_fn = module.getFunction("itoa");
  if (!itoa_fn) {
    auto *i64Ty = llvm::Type::getInt64Ty(ctx);
    auto *i8PtrTy = llvm::Type::getInt8Ty(ctx)->getPointerTo();
    llvm::FunctionType *itoaType =
        llvm::FunctionType::get(i8PtrTy, {i64Ty, i8PtrTy}, false);
    itoa_fn = llvm::Function::Create(itoaType, llvm::Function::ExternalLinkage,
                                     "itoa", module);
  }

  llvm::Function *strlen_fn = module.getFunction("strlen");
  if (!strlen_fn) {
    auto *i8PtrTy = llvm::Type::getInt8Ty(ctx)->getPointerTo();
    llvm::FunctionType *strlenType =
        llvm::FunctionType::get(llvm::Type::getInt64Ty(ctx), {i8PtrTy}, false);
    strlen_fn = llvm::Function::Create(
        strlenType, llvm::Function::ExternalLinkage, "strlen", module);
  }

  std::string final_str;

  llvm::Function *write_fn = module.getFunction("write");
  if (!write_fn) {
    llvm::FunctionType *write_type =
        llvm::FunctionType::get(llvm::Type::getInt64Ty(ctx),
                                {llvm::Type::getInt32Ty(ctx),
                                 llvm::Type::getInt8Ty(ctx)->getPointerTo(),
                                 llvm::Type::getInt64Ty(ctx)},
                                false);
    write_fn = llvm::Function::Create(
        write_type, llvm::Function::ExternalLinkage, "write", module);
  }

  for (size_t i = 0; i < size; ++i) {
    llvm::Value *arg_val = args[i]->codegen(ctx, builder, namedValues);
    if (!arg_val)
      return nullptr;

    llvm::Type *argType = arg_val->getType();
    llvm::Value *strPtr = nullptr;
    llvm::Value *strLen = nullptr;

    // Case 1: Constant string (global)
    if (auto *global = llvm::dyn_cast<llvm::GlobalVariable>(arg_val)) {
      if (auto *data = llvm::dyn_cast<llvm::ConstantDataArray>(
              global->getInitializer())) {
        std::string interpreted = interpretEscapes(data->getAsCString().str());

        // Remove quotes if present
        if (interpreted.size() >= 2 && interpreted.front() == '"' &&
            interpreted.back() == '"') {
          interpreted = interpreted.substr(1, interpreted.size() - 2);
        }

        strPtr = builder.CreateGlobalStringPtr(interpreted);
        strLen = builder.getInt64(interpreted.size());
      }
    }

    // Case 2: Integer or non-string value (e.g., variable like i)
    if (!strPtr) {
      // Allocate buffer for itoa
      llvm::Value *buf =
          builder.CreateAlloca(llvm::ArrayType::get(builder.getInt8Ty(), 21));
      llvm::Value *bufPtr =
          builder.CreatePointerCast(buf, llvm::Type::getInt8Ty(ctx)->getPointerTo());

      // Convert value to i64 if needed
      if (!arg_val->getType()->isIntegerTy(64)) {
        arg_val =
            builder.CreateIntCast(arg_val, llvm::Type::getInt64Ty(ctx), true);
      }

      strPtr = builder.CreateCall(itoa_fn, {arg_val, bufPtr});
      strLen = builder.CreateCall(strlen_fn, {strPtr});
    }

    // Emit write(fd, strPtr, strLen)
    builder.CreateCall(write_fn, {fd_val, strPtr, strLen});

    // Optional space between arguments
    if (i + 1 < size) {
      llvm::Value *spaceStr = builder.CreateGlobalStringPtr(" ");
      builder.CreateCall(write_fn, {fd_val, spaceStr, builder.getInt64(1)});
    }
  }

  if (is_ln) {
    llvm::Value *newline = builder.CreateGlobalStringPtr("\n");
    builder.CreateCall(write_fn, {fd_val, newline, builder.getInt64(1)});
  }

  llvm::Value *str_ptr = builder.CreateGlobalStringPtr(final_str);
  llvm::Value *len_val = builder.getInt64(final_str.size());

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
  (void)module;
  return expr->codegen(ctx, builder, namedValues); // discard result
}

llvm::Value *
VarStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                 llvm::Module &module,
                 std::map<std::string, llvm::Value *> &namedValues) const {
  (void)module;
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
  (void)module;
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

llvm::Value *
LoopStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                  llvm::Module &module,
                  std::map<std::string, llvm::Value *> &namedValues) const {
  llvm::Function *function = builder.GetInsertBlock()->getParent();

  // preHeaderBB
  //   └─> condBB ─┬─> bodyBB ─┬─> stepBB ─┬─> condBB
  //               │           └───────────┘
  //               └────────────> afterBB

  llvm::BasicBlock *preHeaderBB = builder.GetInsertBlock();
  llvm::BasicBlock *condBB =
      llvm::BasicBlock::Create(ctx, "loop.cond", function);
  llvm::BasicBlock *bodyBB =
      llvm::BasicBlock::Create(ctx, "loop.body", function);
  llvm::BasicBlock *stepBB =
      llvm::BasicBlock::Create(ctx, "loop.step", function);
  llvm::BasicBlock *afterBB =
      llvm::BasicBlock::Create(ctx, "loop.after", function);

  if (is_for && init)
    init->codegen(ctx, builder, namedValues);

  builder.CreateBr(condBB);

  builder.SetInsertPoint(condBB);
  llvm::Value *cond_value = nullptr;
  if (condition) {
    cond_value = condition->codegen(ctx, builder, namedValues);
    if (!cond_value)
      return nullptr;

    cond_value = builder.CreateICmpNE(
        cond_value, llvm::ConstantInt::get(cond_value->getType(), 0),
        "loopcond");
  } else {
    cond_value = llvm::ConstantInt::getTrue(ctx);
  }

  builder.CreateCondBr(cond_value, bodyBB, afterBB);

  builder.SetInsertPoint(bodyBB);
  if (block) {
    block->codegen(ctx, builder, module, namedValues);
  }
  builder.CreateBr(stepBB); // Only one terminator

  builder.SetInsertPoint(stepBB);
  if (optional) {
    optional->codegen(ctx, builder, namedValues);
  }
  builder.CreateBr(condBB); // Only one terminator

  builder.SetInsertPoint(afterBB);

  return nullptr;
}

llvm::Value *
IfStmt::codegen(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder,
                llvm::Module &module,
                std::map<std::string, llvm::Value *> &namedValues) const {
  llvm::Function *function = builder.GetInsertBlock()->getParent();

  llvm::BasicBlock *thenBB =
      llvm::BasicBlock::Create(ctx, "if.then", function);
  llvm::BasicBlock *elseBB =
      llvm::BasicBlock::Create(ctx, "if.else", function);
  llvm::BasicBlock *afterBB =
      llvm::BasicBlock::Create(ctx, "if.after", function);

  llvm::Value *cond_value = condition->codegen(ctx, builder, namedValues);
  if (!cond_value)
    return nullptr;

  cond_value = builder.CreateICmpNE(
      cond_value, llvm::ConstantInt::get(cond_value->getType(), 0),
      "ifcond");

  builder.CreateCondBr(cond_value, thenBB, elseBB);

  // --- Then block ---
  builder.SetInsertPoint(thenBB);
  if (block)
    block->codegen(ctx, builder, module, namedValues);
  if (!builder.GetInsertBlock()->getTerminator())
    builder.CreateBr(afterBB); // only jump if not already returned

  // --- Else block ---
  builder.SetInsertPoint(elseBB);
  if (else_block)
    else_block->codegen(ctx, builder, module, namedValues);
  if (!builder.GetInsertBlock()->getTerminator())
    builder.CreateBr(afterBB); // same here

  // --- After block ---
  builder.SetInsertPoint(afterBB);
  return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(ctx));
}
