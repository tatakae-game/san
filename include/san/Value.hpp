#pragma once

#include <san/Name.hpp>
#include <san/Type.hpp>

namespace San
{
class Value : public Name
{
public:
    Type *type;
    llvm::Value *ref;

    Value *calling_variable = nullptr;

    bool is_alloca = false;

    Value(const std::string &name_, Type *type_, llvm::Value *ref_, const bool &is_alloca_ = false) : Name(name_), type(type_), ref(ref_), is_alloca(is_alloca_)
    {
    }

    virtual llvm::Value *get_ref()
    {
        return this->ref;
    }

    std::string to_string()
    {
        std::string str;
        llvm::raw_string_ostream stream(str);
        this->ref->print(stream);

        return stream.str();
    }

    bool is_allocated_variable()
    {
        auto ref = this->get_ref();
        return !!llvm::isa<llvm::AllocaInst>(ref) || !!llvm::isa<llvm::GlobalVariable>(ref);
    }

    Value *call(llvm::IRBuilder<> &builder, std::unique_ptr<llvm::Module> &module, std::vector<Value *> args = {});

    void store(Value *value, llvm::IRBuilder<> &builder, std::unique_ptr<llvm::Module> &module, const bool &overwrite_reference = false)
    {
        auto lvalue = this;
        auto lvalue_type = lvalue->type;

        if (lvalue->is_alloca && lvalue_type->is_reference && !overwrite_reference)
        {
            lvalue = lvalue->load_reference(builder);
        }

        if (this->type->is_struct() && value->type->is_struct())
        {
            auto i8ptr = Type::i8(builder.getContext())->pointer();

            auto rvalue = value->cast(i8ptr, builder, false);
            lvalue = lvalue->cast(i8ptr, builder, false);

            auto size = llvm::ConstantInt::get(llvm::Type::getInt64Ty(builder.getContext()), lvalue->type->size(module));

            builder.CreateMemCpy(lvalue->get_ref(), llvm::Align(1), rvalue->get_ref(), llvm::Align(1), llvm::cast<llvm::Value>(size));
        }
        else if (this->type->is_array() && value->type->is_array())
        {
            auto i8ptr = Type::i8(builder.getContext())->pointer();

            auto rvalue = value->cast(i8ptr, builder, false);
            lvalue = lvalue->cast(i8ptr, builder, false);

            auto size = llvm::ConstantInt::get(llvm::Type::getInt64Ty(builder.getContext()), lvalue->type->size(module));

            builder.CreateMemCpy(lvalue->get_ref(), this->get_ref()->getPointerAlignment(module->getDataLayout()), rvalue->get_ref(), value->get_ref()->getPointerAlignment(module->getDataLayout()), llvm::cast<llvm::Value>(size));
        }
        else
        {
            if (!overwrite_reference)
            {
                value = value->load_alloca_and_reference(builder);
            }
            else if (value->is_alloca && value->type->is_reference)
            {
                value = value->load(builder, false);
            }

            auto rvalue = value->cast(lvalue_type, builder, value->is_alloca);
            builder.CreateStore(rvalue->get_ref(), lvalue->get_ref());
        }
    }

    virtual Value *load(llvm::IRBuilder<> &builder, const bool &load_reference = false)
    {
        if (!this->type->is_pointer() && !this->is_alloca)
        {
            return this;
        }

        if (this->type->is_array())
        {
            if (this->is_alloca && load_reference && this->type->is_reference)
            {
                auto value = this->load_reference(builder);
                return value->load_array(builder);
            }
            else
            {
                return this->load_array(builder);
            }
        }

        auto value = builder.CreateLoad(this->get_ref());

        if (this->is_alloca && load_reference && this->type->is_reference)
        {
            value = builder.CreateLoad(value);
        }

        auto type = this->type;

        if (!this->is_alloca && load_reference && this->type->is_reference)
        {
            type = type->get_base(false);
        }

        return new Value(this->name + ".load", type, value);
    }

    Value *load_array(llvm::IRBuilder<> &builder)
    {
        auto ref = builder.CreateConstInBoundsGEP2_64(this->get_ref(), 0, 0, "");
        return new Value(this->name + ".load", this->type, ref);
    }

    Value *load_reference(llvm::IRBuilder<> &builder)
    {
        if (this->type->is_reference)
        {
            auto value = builder.CreateLoad(this->get_ref());
            auto type = this->type->get_base(false);

            return new Value(this->name + ".load", type, value);
        }

        return this;
    }

    Value *load_alloca_and_reference(llvm::IRBuilder<> &builder)
    {
        auto value = this;

        if (value->is_alloca)
        {
            if (value->type->is_reference && value->type->base->is_array())
            {
                value = value->load_reference(builder);
            }

            if (value->type->is_array())
            {
                return this->load_array(builder);
            }

            auto ref = builder.CreateLoad(this->get_ref());
            value = new Value(this->name + ".load", value->type, ref);
        }

        return value->load_reference(builder);
    }

    Value *gep(Value *index, llvm::IRBuilder<> &builder)
    {
        auto index_value = index->cast(Type::i64(builder.getContext()), builder)->get_ref();
        return this->gep(index_value, builder);
    }

    Value *gep(const size_t &index, llvm::IRBuilder<> &builder)
    {
        auto index_value = llvm::ConstantInt::get(Type::llvm_i64(builder.getContext()), index);
        return this->gep(index_value, builder);
    }

    Value *gep(llvm::Value *index, llvm::IRBuilder<> &builder)
    {
        auto &context = builder.getContext();

        auto value = this->load_alloca_and_reference(builder);
        std::vector<llvm::Value *> idxs;

        if (!this->type->is_pointer())
        {
            idxs.push_back(llvm::ConstantInt::get(Type::llvm_i64(context), 0));
        }

        idxs.push_back(index);

        auto pointer = builder.CreateInBoundsGEP(value->get_ref(), idxs, "idx");
        auto type = value->type->get_base(false);

        return new Value("idx", type, pointer, true);
    }

    Value *struct_gep(const std::string &name, Type *property_type, const size_t &index, llvm::IRBuilder<> &builder)
    {
        auto &context = builder.getContext();

        std::vector<llvm::Value *> idxs = {
            llvm::ConstantInt::get(Type::llvm_i32(context), 0),
            llvm::ConstantInt::get(Type::llvm_i32(context), index),
        };

        auto value = builder.CreateInBoundsGEP(this->get_ref(), idxs, name);

        return new Value(name, property_type, value, true);
    }

    Value *struct_cast(Type *dest, const size_t &padding, llvm::IRBuilder<> &builder)
    {
        auto type = this->type;

        if (type->equals(dest))
        {
            return this;
        }

        auto &context = builder.getContext();

        auto bytes = builder.CreateBitCast(this->get_ref(), Type::llvm_i8(builder.getContext())->getPointerTo());

        std::vector<llvm::Value *> idxs = {
            llvm::ConstantInt::get(Type::llvm_i64(context), padding),
        };

        auto value = builder.CreateInBoundsGEP(bytes, idxs, "idx");
        value = builder.CreateBitCast(value, dest->pointer()->get_ref());

        return new Value(this->name, dest, value, true);
    }

    Value *union_cast(Type *dest, llvm::IRBuilder<> &builder)
    {
        auto value = builder.CreateBitCast(this->get_ref(), dest->get_ref()->getPointerTo());
        return new Value(this->name, dest, value, true);
    }

    virtual Value *cast(Type *dest, llvm::IRBuilder<> &builder, const bool &load = true)
    {
        auto value = this;
        auto type = value->type;

        if (type->is_reference)
        {
            if (!dest->is_reference)
            {
                value = value->load(builder, true);
            }
        }
        else if (dest->is_reference)
        {
            dest = dest->base;
        }
        else if (load)
        {
            value = value->load_alloca_and_reference(builder);
        }

        if (value->is_alloca && value->type->is_reference)
        {
            value = value->load(builder, false);
        }

        auto ref = value->get_ref();
        type = value->type;

        if (type->is_integer())
        {
            if (dest->is_integer())
            {
                auto lbits = dest->ref->getIntegerBitWidth();
                auto rbits = type->ref->getIntegerBitWidth();

                if (lbits != rbits)
                {
                    bool is_signed = type->is_signed;

                    if (is_signed)
                    {
                        ref = builder.CreateSExtOrTrunc(ref, dest->ref);
                    }
                    else
                    {
                        ref = builder.CreateZExtOrTrunc(ref, dest->ref);
                    }
                }

                if (dest->is_boolean())
                {
                    ref = builder.CreateICmpNE(ref, llvm::ConstantInt::get(ref->getType(), 0));
                }
            }
            else if (dest->is_floating_point())
            {
                if (type->is_signed)
                {
                    ref = builder.CreateSIToFP(ref, dest->ref);
                }
                else
                {
                    ref = builder.CreateUIToFP(ref, dest->ref);
                }
            }
            else if (dest->is_pointer())
            {
                ref = builder.CreateIntToPtr(ref, dest->ref);
            }

            return new Value(this->name, dest, ref);
        }
        else if (type->is_double())
        {
            if (dest->is_float())
            {
                ref = builder.CreateFPTrunc(ref, dest->ref);
            }
            else if (dest->is_integer())
            {
                if (dest->is_signed)
                {
                    ref = builder.CreateFPToSI(ref, dest->ref);
                }
                else
                {
                    ref = builder.CreateFPToUI(ref, dest->ref);
                }
            }

            return new Value(this->name, dest, ref);
        }
        else if (type->is_float())
        {
            if (dest->is_double())
            {
                ref = builder.CreateFPExt(ref, dest->ref);
            }
            else if (dest->is_integer())
            {
                if (dest->is_signed)
                {
                    ref = builder.CreateFPToSI(ref, dest->ref);
                }
                else
                {
                    ref = builder.CreateFPToUI(ref, dest->ref);
                }
            }

            return new Value(this->name, dest, ref);
        }
        else if (type->is_pointer())
        {
            if (dest->is_boolean())
            {
                auto pointer_type = reinterpret_cast<llvm::PointerType *>(type->ref);
                ref = builder.CreateICmpNE(ref, llvm::ConstantPointerNull::get(pointer_type));
            }
            else if (dest->is_integer())
            {
                ref = builder.CreatePtrToInt(ref, dest->ref);
            }
            else if (dest->is_pointer() && !dest->equals(type))
            {
                ref = builder.CreateBitCast(ref, dest->ref);
            }

            return new Value(this->name, dest, ref);
        }

        return value;
    }

    static Value *add(llvm::IRBuilder<> &builder, Value *lvalue, Value *rvalue)
    {
        auto ltype = lvalue->type->behind_reference();
        auto rtype = rvalue->type->behind_reference();

        if (ltype->is_integer())
        {
            lvalue = lvalue->load_alloca_and_reference(builder);
            rvalue = rvalue->cast(lvalue->type, builder);

            auto value = builder.CreateAdd(lvalue->get_ref(), rvalue->get_ref());
            return new Value("add", lvalue->type, value);
        }
        else if (ltype->is_floating_point())
        {
            lvalue = lvalue->load_alloca_and_reference(builder);
            rvalue = rvalue->cast(lvalue->type, builder);

            auto value = builder.CreateFAdd(lvalue->get_ref(), rvalue->get_ref());
            return new Value("add", lvalue->type, value);
        }
        else if (ltype->is_pointer() && rtype->is_integer())
        {
            lvalue = lvalue->load_alloca_and_reference(builder);
            rvalue = rvalue->load_alloca_and_reference(builder);

            auto value = lvalue->gep(rvalue, builder);
            value->is_alloca = false;
            value->type = ltype;

            return value;
        }

        return nullptr;
    }

    Value *add(std::unique_ptr<llvm::Module> &module, llvm::IRBuilder<> &builder, Value *rvalue)
    {
        if (auto result = Value::add(builder, this->load_alloca_and_reference(builder), rvalue->load_alloca_and_reference(builder)))
        {
            this->store(result, builder, module);
            return this;
        }

        return nullptr;
    }

    static Value *sub(llvm::IRBuilder<> &builder, Value *lvalue, Value *rvalue)
    {
        auto ltype = lvalue->type->behind_reference();
        auto rtype = rvalue->type->behind_reference();

        if (ltype->is_integer())
        {
            lvalue = lvalue->load_alloca_and_reference(builder);
            rvalue = rvalue->cast(lvalue->type, builder);

            auto value = builder.CreateSub(lvalue->get_ref(), rvalue->get_ref());
            return new Value("sub", lvalue->type, value);
        }
        else if (ltype->is_floating_point())
        {
            lvalue = lvalue->load_alloca_and_reference(builder);
            rvalue = rvalue->cast(lvalue->type, builder);

            auto value = builder.CreateFSub(lvalue->get_ref(), rvalue->get_ref());
            return new Value("sub", lvalue->type, value);
        }
        else if (ltype->is_pointer() && rtype->is_integer())
        {
            lvalue = lvalue->load_alloca_and_reference(builder);
            rvalue = rvalue->load_alloca_and_reference(builder);

            return lvalue->gep(rvalue, builder);
        }

        return nullptr;
    }

    Value *sub(std::unique_ptr<llvm::Module> &module, llvm::IRBuilder<> &builder, Value *rvalue)
    {
        if (auto result = Value::sub(builder, this, rvalue))
        {
            this->store(result, builder, module);
            return result;
        }

        return nullptr;
    }

    Value *not_equal(llvm::IRBuilder<> &builder, Value *rvalue)
    {
        auto type = Type::i1(builder.getContext());

        if (this->type->is_integer() || this->type->is_pointer())
        {
            auto value = builder.CreateICmpNE(this->get_ref(), rvalue->get_ref());
            return new Value("ne", type, value);
        }
        else if (this->type->is_floating_point())
        {
            auto value = builder.CreateFCmpUNE(this->get_ref(), rvalue->get_ref());
            return new Value("ne", type, value);
        }

        return nullptr;
    }
};
} // namespace San
