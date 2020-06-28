#pragma once

#include <san/Name.hpp>

#include <llvm/IR/IRBuilder.h>

#include <cstdlib>
#include <iostream>
#include <limits>

namespace San
{
class Type : public Name
{
public:
    static constexpr size_t NOT_COMPATIBLE = std::numeric_limits<size_t>::max();

public:
    bool is_constant = false;
    bool is_signed = true;
    bool is_reference = false;

    llvm::Type *ref;
    Type *base = nullptr;

    Type(const std::string &name,
         llvm::Type *ref_,
         Type *base_ = nullptr,
         const bool &is_signed_ = true,
         const bool &is_constant_ = false,
         const bool &is_reference_ = false) : Name(name),
                                              is_constant(is_constant_),
                                              is_signed(is_signed_),
                                              is_reference(is_reference_),
                                              ref(ref_),
                                              base(base_)
    {
    }

    virtual llvm::Type *get_ref()
    {
        return this->ref;
    }

    Type *behind_reference()
    {
        if (this->is_reference)
        {
            return this->base;
        }

        return this;
    }

    std::string to_string()
    {
        std::string str;
        llvm::raw_string_ostream stream(str);
        this->ref->print(stream);

        return stream.str();
    }

    Type *pointer()
    {
        auto ref = this->get_ref();

        if (this->is_void())
        {
            ref = Type::llvm_i8(ref->getContext());
        }

        return new Type(this->name + "*", ref->getPointerTo(), this);
    }

    Type *reference()
    {
        auto pointer_type = this->pointer();
        pointer_type->is_reference = true;

        return pointer_type;
    }

    Type *array(const size_t &size)
    {
        auto ref = this->get_ref();
        return new Type(this->name + "[" + std::to_string(size) + "]", llvm::ArrayType::get(ref, size), this);
    }

    Type *copy() const
    {
        return new Type(*this);
    }

    llvm::Constant *default_value()
    {
        if (this->is_integer())
        {
            return llvm::ConstantInt::get(this->ref, 0);
        }
        else if (this->is_floating_point())
        {
            return llvm::ConstantFP::get(this->ref, 0.0);
        }
        else
        {
            return llvm::ConstantPointerNull::get(reinterpret_cast<llvm::PointerType *>(this->ref));
        }
    }

    virtual size_t size(std::unique_ptr<llvm::Module> &module)
    {
        if (this->is_pointer())
        {
            return module->getDataLayout().getPointerSize();
        }

        if (this->is_array())
        {
            return this->base->size(module) * this->ref->getArrayNumElements();
        }

        return this->ref->getScalarSizeInBits() / 8;
    }

    Type *get_base(const bool &root = true)
    {
        if (this->base == nullptr)
        {
            return this;
        }
        else if (!root)
        {
            return this->base;
        }

        return this->base->get_base();
    }

    inline bool is_void() const
    {
        return this->ref->isVoidTy();
    }

    inline bool is_integer() const
    {
        return this->ref->isIntegerTy();
    }

    inline bool is_integer(const unsigned int &bits) const
    {
        return this->ref->isIntegerTy(bits);
    }

    inline bool is_double() const
    {
        return this->ref->isDoubleTy();
    }

    inline bool is_float() const
    {
        return this->ref->isFloatTy();
    }

    inline bool is_floating_point() const
    {
        return this->ref->isFloatingPointTy();
    }

    inline bool is_pointer() const
    {
        return this->ref->isPointerTy();
    }

    inline bool is_array() const
    {
        return this->ref->isArrayTy();
    }

    inline bool is_function() const
    {
        return this->ref->isFunctionTy();
    }

    inline bool is_boolean() const
    {
        return this->ref->isIntegerTy(1);
    }

    inline bool is_struct() const
    {
        return this->ref->isStructTy();
    }

    static Type *voidt(llvm::LLVMContext &context)
    {
        return new Type("void", Type::llvm_void(context));
    }

    static Type *i1(llvm::LLVMContext &context)
    {
        return new Type("i1", Type::llvm_i1(context));
    }

    static Type *i8(llvm::LLVMContext &context, const bool &is_signed = true)
    {
        return new Type("i8", Type::llvm_i8(context), nullptr, is_signed);
    }

    static Type *i16(llvm::LLVMContext &context, const bool &is_signed = true)
    {
        return new Type("i16", Type::llvm_i16(context), nullptr, is_signed);
    }

    static Type *i32(llvm::LLVMContext &context, const bool &is_signed = true)
    {
        return new Type("i32", Type::llvm_i32(context), nullptr, is_signed);
    }

    static Type *i64(llvm::LLVMContext &context, const bool &is_signed = true)
    {
        return new Type("i64", Type::llvm_i64(context), nullptr, is_signed);
    }

    static Type *f32(llvm::LLVMContext &context)
    {
        return new Type("f32", Type::llvm_f32(context));
    }

    static Type *f64(llvm::LLVMContext &context)
    {
        return new Type("f64", Type::llvm_f64(context));
    }

    static llvm::Type *llvm_void(llvm::LLVMContext &context)
    {
        return llvm::Type::getVoidTy(context);
    }

    static llvm::Type *llvm_i1(llvm::LLVMContext &context)
    {
        return llvm::Type::getInt1Ty(context);
    }

    static llvm::Type *llvm_i8(llvm::LLVMContext &context)
    {
        return llvm::Type::getInt8Ty(context);
    }

    static llvm::Type *llvm_i16(llvm::LLVMContext &context)
    {
        return llvm::Type::getInt16Ty(context);
    }

    static llvm::Type *llvm_i32(llvm::LLVMContext &context)
    {
        return llvm::Type::getInt32Ty(context);
    }

    static llvm::Type *llvm_i64(llvm::LLVMContext &context)
    {
        return llvm::Type::getInt64Ty(context);
    }

    static llvm::Type *llvm_f32(llvm::LLVMContext &context)
    {
        return llvm::Type::getFloatTy(context);
    }

    static llvm::Type *llvm_f64(llvm::LLVMContext &context)
    {
        return llvm::Type::getDoubleTy(context);
    }

    inline bool equals(Type *right)
    {
        return Type::equals(this, right);
    }

    static bool equals(Type *left, Type *right)
    {
        if (left->is_reference)
        {
            return equals(left->base, right);
        }

        if (right->is_reference)
        {
            return equals(left, right->base);
        }

        if (left->is_pointer())
        {
            if (!right->is_pointer() && !right->is_array())
            {
                return false;
            }

            return Type::equals(left->base, right->base);
        }

        if (left->is_integer() != right->is_integer())
        {
            return false;
        }
        else if (left->is_integer() && right->is_integer())
        {
            if (left->is_signed != right->is_signed)
            {
                return false;
            }

            return left->get_ref()->getIntegerBitWidth() == right->get_ref()->getIntegerBitWidth();
        }

        return Type::equals(left->ref, right->ref);
    }

    static bool equals(const llvm::Type *left, const llvm::Type *right)
    {
        auto left_ptr = llvm::dyn_cast<llvm::PointerType>(left);
        auto right_ptr = llvm::dyn_cast<llvm::PointerType>(right);

        if (left == right)
            return true;

        // if (left->getTypeID() == right->getTypeID())
        //     return true;

        switch (left->getTypeID())
        {
        case llvm::Type::IntegerTyID:
            return llvm::cast<llvm::IntegerType>(left)->getBitWidth() == llvm::cast<llvm::IntegerType>(right)->getBitWidth();

        // left == right would have returned true earlier, because types are uniqued.
        case llvm::Type::VoidTyID:
        case llvm::Type::FloatTyID:
        case llvm::Type::DoubleTyID:
        case llvm::Type::X86_FP80TyID:
        case llvm::Type::FP128TyID:
        case llvm::Type::PPC_FP128TyID:
        case llvm::Type::LabelTyID:
        case llvm::Type::MetadataTyID:
        case llvm::Type::TokenTyID:
            return true;

        case llvm::Type::PointerTyID:
            if (right_ptr)
            {
                return Type::equals(left_ptr->getElementType(), right_ptr->getElementType());
            }
            else if (auto right_sequential = llvm::dyn_cast<llvm::SequentialType>(right))
            {
                return Type::equals(left_ptr->getElementType(), right_sequential->getElementType());
            }

            return false;

        case llvm::Type::StructTyID:
        {
            auto left_struct = llvm::cast<llvm::StructType>(left);
            auto right_struct = llvm::cast<llvm::StructType>(right);

            if (left_struct->getNumElements() != right_struct->getNumElements())
                return false;

            if (left_struct->isPacked() != right_struct->isPacked())
                return false;

            for (unsigned i = 0, e = left_struct->getNumElements(); i != e; ++i)
            {
                if (!Type::equals(left_struct->getElementType(i), right_struct->getElementType(i)))
                    return false;
            }

            return true;
        }

        case llvm::Type::FunctionTyID:
        {
            auto left_function = llvm::cast<llvm::FunctionType>(left);
            auto right_function = llvm::cast<llvm::FunctionType>(right);

            if (left_function->getNumParams() != right_function->getNumParams())
                return false;

            if (left_function->isVarArg() != right_function->isVarArg())
                return false;

            if (!Type::equals(left_function->getReturnType(), right_function->getReturnType()))
                return false;

            for (unsigned i = 0, e = left_function->getNumParams(); i != e; ++i)
            {
                if (!Type::equals(left_function->getParamType(i), right_function->getParamType(i)))
                    return false;
            }

            return true;
        }

        case llvm::Type::ArrayTyID:
        case llvm::Type::VectorTyID:
        {
            auto left_sequential = llvm::cast<llvm::SequentialType>(left);

            if (auto right_sequential = llvm::dyn_cast<llvm::SequentialType>(right))
            {
                if (left_sequential->getNumElements() != right_sequential->getNumElements())
                    return false;

                return Type::equals(left_sequential->getElementType(), right_sequential->getElementType());
            }
            else if (auto right_pointer = llvm::dyn_cast<llvm::PointerType>(right))
            {
                return Type::equals(left_sequential->getElementType(), right_pointer->getElementType());
            }

            return false;
        }

        default:
            return false;
        }
    }

    inline size_t compatibility(Type *right)
    {
        return Type::compatibility(this, right);
    }

    static size_t compatibility(Type *left, Type *right)
    {
        if (left->is_reference)
        {
            return compatibility(left->base, right);
        }

        if (right->is_reference)
        {
            return compatibility(left, right->base);
        }

        if (left->is_pointer())
        {
            if (!right->is_pointer() && !right->is_array())
            {
                return NOT_COMPATIBLE;
            }

            return Type::compatibility(left->base, right->base);
        }

        if (left->is_integer())
        {
            if (!right->is_integer())
            {
                return NOT_COMPATIBLE;
            }

            size_t score = 0;

            if (left->is_signed != right->is_signed)
            {
                score += 10;
            }

            auto lbits = left->get_ref()->getIntegerBitWidth() / 8;
            auto rbits = right->get_ref()->getIntegerBitWidth() / 8;

            if (lbits > rbits)
            {
                return score + (lbits - rbits);
            }
            else
            {
                return score + (rbits - lbits);
            }
        }

        return Type::equals(left->ref, right->ref) ? 0UL : NOT_COMPATIBLE;
    }
};
} // namespace San
