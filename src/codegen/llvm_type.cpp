#include "../ast/type.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

llvm::Type *SymbolType::codegen(llvm::LLVMContext &ctx) const {
  // NOTE: Handle strings
  if (name == "uint" || name == "int")
    return llvm::Type::getInt64Ty(ctx);
  if (name == "float")
    return llvm::Type::getDoubleTy(ctx);
  if (name == "char")
    return llvm::Type::getInt8Ty(ctx);
  if (name == "str")
    return llvm::Type::getInt8Ty(ctx)->getPointerTo();
  if (name == "bool")
    return llvm::Type::getInt1Ty(ctx);
  if (name == "nil")
    return llvm::Type::getVoidTy(ctx);

  throw std::runtime_error("Unknown type: " + name);
}
