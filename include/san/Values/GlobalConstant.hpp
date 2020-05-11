#pragma once

#include <san/Values/Constant.hpp>

#include <llvm/IR/IRBuilder.h>

namespace San::Values
{
class GlobalConstant : public Constant
{
public:
    llvm::GlobalVariable *global = nullptr;

    GlobalConstant(const std::string &name, Type *type, llvm::GlobalVariable *ref) : Constant(name, type, ref->getInitializer()), global(ref)
    {
    }

    static GlobalConstant *create(const std::string &name, Type *type, llvm::Constant *constant, std::unique_ptr<llvm::Module> &module)
    {
        auto global = new llvm::GlobalVariable(*module, type->get_ref(), false, llvm::GlobalValue::PrivateLinkage, constant, ".str");
        global->setAlignment(1);

        return new GlobalConstant(name, type, global);
    }

    llvm::GlobalVariable *get_ref() override
    {
        return this->global;
    }
};
} // namespace San::Values