#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <map>

class CodegenContext {
public:
  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  std::unique_ptr<llvm::Module> module;

  std::map<std::string, llvm::Value *> namedValues;

  CodegenContext(const std::string &moduleName)
      : builder(context),
        module(std::make_unique<llvm::Module>(moduleName, context)) {}
};
