#pragma once

#include "runtime/SanParserBaseVisitor.h"

#include "ParserErrorListener.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/Transforms/Utils/Evaluator.h>

#include <san/Debugger.hpp>
#include <san/Environment.hpp>
#include <san/Helpers.hpp>

#include <san/Alias.hpp>
#include <san/AssemblyOperand.hpp>
#include <san/Attributes.hpp>
#include <san/NameArray.hpp>
#include <san/Namespace.hpp>
#include <san/Scope.hpp>
#include <san/ScopeStack.hpp>
#include <san/StatementStatus.hpp>
#include <san/Types/ClassType.hpp>
#include <san/Types/GenericAlias.hpp>
#include <san/Types/GenericClassType.hpp>
#include <san/Types/GenericFunctionType.hpp>
#include <san/Types/UnionType.hpp>
#include <san/Values/Function.hpp>
#include <san/Values/GlobalConstant.hpp>
#include <san/Values/GlobalVariable.hpp>
#include <san/Values/Variable.hpp>

#include <san/Exceptions/ExpressionHasNotClassTypeException.hpp>
#include <san/Exceptions/ImportException.hpp>
#include <san/Exceptions/InvalidInputConstraintException.hpp>
#include <san/Exceptions/InvalidLeftValueException.hpp>
#include <san/Exceptions/InvalidRangeException.hpp>
#include <san/Exceptions/InvalidRightValueException.hpp>
#include <san/Exceptions/InvalidTypeException.hpp>
#include <san/Exceptions/InvalidValueException.hpp>
#include <san/Exceptions/MultipleInstancesException.hpp>
#include <san/Exceptions/NoFunctionMatchException.hpp>
#include <san/Exceptions/NotAClassException.hpp>
#include <san/Exceptions/NotAClassOrNamespaceException.hpp>
#include <san/Exceptions/NotAGenericException.hpp>
#include <san/Exceptions/NotAPointerException.hpp>
#include <san/Exceptions/PropertyNotFoundException.hpp>
#include <san/Exceptions/UnknownNameException.hpp>

#include <san/filesystem.hpp>

#include <regex>

namespace San
{
class Visitor
{
public:
    Environment env;
    std::vector<std::string> include_paths;

    ScopeStack scopes;
    std::stack<fs::path> files;
    std::vector<fs::path> imported;

    size_t generating_properties_stack = 0;

    Visitor(const std::string &target_os,
            const std::string &target_arch,
            const std::vector<std::string> &include_paths_ = {}) : env("output", target_os, target_arch),
                                                                   include_paths(include_paths_),
                                                                   scopes(this->env)
    {
        this->include_paths.push_back(Environment::get_std_directory().u8string());
    }

    void from_file(std::string path)
    {
        if (!this->files.empty())
        {
            if (Helpers::starts_with(path, "./"))
            {
                auto from = this->files.top();
                path = from.replace_filename(path).u8string();
            }
            else if (Helpers::starts_with(path, "/"))
            {
                // nothing to do here, this condition exists in case of future implementation
                // the compiler will optimize this anyway
            }
            else
            {
                for (const auto &include_path : include_paths)
                {
                    auto separator = (include_path[include_path.size() - 1] != '/' ? "/" : "");
                    auto fullpath = fs::absolute(include_path + separator + path + ".sn");

                    if (fs::exists(fullpath))
                    {
                        path = fullpath.u8string();
                        break;
                    }
                }
            }
        }

        auto fullpath = fs::absolute(path);

        if (!fs::exists(fullpath) && !Helpers::ends_with(fullpath.u8string(), ".sn"))
        {
            fullpath += ".sn";
        }

        if (!fs::exists(fullpath))
        {
            throw FileNotFoundException();
        }

        fullpath = fs::canonical(fullpath);

        if (std::find(imported.begin(), imported.end(), fullpath) != imported.end())
        {
            return;
        }

        imported.push_back(fullpath);

        std::ifstream stream;
        stream.open(fullpath);

        files.push(fullpath);

        auto input = new ANTLRInputStream(stream);
        auto lexer = new SanLexer(input);
        auto tokens = new CommonTokenStream(lexer);
        auto parser = new SanParser(tokens);
        // parser->removeErrorListeners();

        // auto error_listener = new ParserErrorListener(this->env.debugger);
        // parser->addErrorListener(error_listener);

        SanParser::InstructionsContext *context = parser->instructions();

        this->visitInstructions(context);

        files.pop();
    }

    void visitInstructions(SanParser::InstructionsContext *context)
    {
        this->visitStatements(context->statement());
    }

    StatementStatus visitStatements(const std::vector<SanParser::StatementContext *> &statements)
    {
        for (const auto &statement : statements)
        {
            auto value = this->visitStatement(statement);

            if (statement->returnStatement())
            {
                return StatementStatus::Returned;
            }
            else if (statement->breakStatement())
            {
                return StatementStatus::Breaked;
            }
            else if (auto block = dynamic_cast<Block *>(value))
            {
                if (block->status == StatementStatus::Returned || block->status == StatementStatus::Breaked)
                {
                    return block->status;
                }
            }
        }

        return StatementStatus::None;
    }

    Name *visitStatement(SanParser::StatementContext *context)
    {
        if (auto function = context->function())
        {
            return this->visitFunction(function);
        }
        else if (auto namespace_statement = context->namespaceStatement())
        {
            return this->visitNamespaceStatement(namespace_statement);
        }
        else if (auto expression = context->expression())
        {
            return this->visitExpression(expression);
        }
        else if (auto body = context->body())
        {
            return this->visitBody(body);
        }
        else if (auto variable_declaration = context->variableDeclaration())
        {
            return this->visitVariableDeclaration(variable_declaration);
        }
        else if (auto return_statement = context->returnStatement())
        {
            this->visitReturnStatement(return_statement);
        }
        else if (auto if_statement = context->ifStatement())
        {
            this->visitIfStatement(if_statement);
        }
        else if (auto while_statement = context->whileStatement())
        {
            this->visitWhileStatement(while_statement);
        }
        else if (auto for_statement = context->forStatement())
        {
            this->visitForStatement(for_statement);
        }
        else if (auto special_class_statement = context->specialClassStatement())
        {
            return this->visitSpecialClassStatement(special_class_statement);
        }
        else if (auto class_statement = context->classStatement())
        {
            return this->visitClassStatement(class_statement);
        }
        else if (auto union_statement = context->unionStatement())
        {
            return this->visitUnionStatement(union_statement);
        }
        else if (auto import_statement = context->importStatement())
        {
            this->visitImportStatement(import_statement);
        }
        else if (auto assembly_statement = context->assemblyStatement())
        {
            this->visitAssemblyStatement(assembly_statement);
        }
        else if (auto alias_statement = context->alias())
        {
            return this->visitAlias(alias_statement);
        }

        return nullptr;
    }

    /**
     * Return value can be a pointer of Function or GenericFunctionType
     */
    Name *visitFunction(SanParser::FunctionContext *context, const bool add_to_scope = true, const bool generate_body = true, Types::ClassType *this_type = nullptr)
    {
        auto scope = this->scopes.top();

        auto attributes = this->visitAttributes(context->attributes());

        if (!attributes.accept_current_target())
            return nullptr;

        Position position;

        if (scope->in_function())
        {
            position = Position::save(scope->builder());
        }

        auto type = this->visitFunctionDeclaration(context->functionDeclaration(), this_type);

        if (auto function_type = dynamic_cast<Types::FunctionType *>(type))
        {
            auto is_extern = !!context->Extern() || function_type->name == "main";
            auto linkage = is_extern ? llvm::GlobalValue::LinkageTypes::ExternalLinkage : llvm::GlobalValue::LinkageTypes::LinkOnceAnyLinkage;

            auto function = new Values::Function(scope->module(), function_type, linkage);

            if (attributes.noinline)
            {
                function->get_ref()->addAttribute(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoInline);
            }

            if (add_to_scope)
            {
                scope->add_name(function->name, function);
            }

            if (generate_body)
            {
                this->generateFunctionBody(context, function);
            }

            position.load(scope->builder());

            return function;
        }
        else if (auto generic_type = dynamic_cast<Types::GenericFunctionType *>(type))
        {
            generic_type->context = context;
        }

        if (add_to_scope)
        {
            scope->add_name(type->name, type);
        }

        position.load(scope->builder());

        return type;
    }

    Values::Function *visitFunction(Types::GenericFunctionType *generic)
    {
        auto scope = this->scopes.top();

        Position position;

        if (scope->in_function())
        {
            position = Position::save(scope->builder());
        }

        auto context = generic->context;

        auto type = this->visitFunctionDeclaration(context->functionDeclaration(), generic->parent, true);

        if (auto function_type = dynamic_cast<Types::FunctionType *>(type))
        {
            auto function = new Values::Function(scope->module(), function_type);
            generic->children.push_back(function);

            this->generateFunctionBody(context, function);

            position.load(scope->builder());

            return function;
        }
    }

    Values::Function *generateFunctionBody(SanParser::FunctionContext *context, Values::Function *base)
    {
        this->scopes.create(base);

        if (auto body = context->body())
        {
            this->visitBody(body, base);
        }

        this->scopes.pop();

        return base;
    }

    /**
     * Return value can be a pointer of FunctionType or GenericFunctionType
     */
    Name *visitFunctionDeclaration(SanParser::FunctionDeclarationContext *context, Types::ClassType *parent, const bool &bypass_generics = false)
    {
        auto scope = this->scopes.top();

        std::string name = "";

        Type *cast_type = nullptr;

        if (auto operator_name = context->overloadableOperator())
        {
            name = operator_name->getText();
        }
        else if (auto type_context = context->castFunctionType())
        {
            cast_type = this->visitCastFunctionType(type_context);

            name = "@cast";
        }
        else if (auto variable_name = context->VariableName())
        {
            name = variable_name->getText();
        }
        else if (context->Destructor())
        {
            name = "@destructor";
        }

        if (!bypass_generics)
        {
            if (auto generics_context = context->classGenerics())
            {
                auto generics = this->visitClassGenerics(generics_context);
                return new Types::GenericFunctionType(Scope::create(scope), name, generics, parent);
            }
        }

        auto args = this->visitFunctionArguments(context->functionArguments());
        auto is_variadic = (context->functionVariadicArgument() != nullptr);

        if (parent != nullptr)
        {
            auto arg = Types::FunctionArgument("this", parent->pointer());
            args.insert(args.begin(), arg);
        }

        Type *return_type = nullptr;

        if (cast_type)
        {
            return_type = cast_type;
        }
        else if (name == "main")
        {
            return_type = Type::i32(scope->context());
        }
        else if (auto type_context = context->type())
        {
            return_type = this->visitType(type_context);
        }
        else
        {
            return_type = Type::voidt(scope->context());
        }

        return Types::FunctionType::create(scope->builder(), scope->module(), name, return_type, args, is_variadic, parent != nullptr);
    }

    Type *visitCastFunctionType(SanParser::CastFunctionTypeContext *context)
    {
        return this->visitType(context->type());
    }

    std::vector<Types::FunctionArgument> visitFunctionArguments(SanParser::FunctionArgumentsContext *context)
    {
        std::vector<Types::FunctionArgument> arguments;

        if (context == nullptr)
            return arguments;

        for (auto argument_context : context->functionArgument())
        {
            auto argument = this->visitFunctionArgument(argument_context);
            arguments.push_back(argument);
        }

        return arguments;
    }

    Types::FunctionArgument visitFunctionArgument(SanParser::FunctionArgumentContext *context)
    {
        auto scope = this->scopes.top();

        std::string name = "";

        if (auto variable_name = context->VariableName())
        {
            name = variable_name->getText();
        }

        auto type = this->visitType(context->type());

        // implement default value

        return Types::FunctionArgument(name, type);
    }

    Values::Function *generateGenericFunction(Types::GenericFunctionType *generic, const std::vector<Type *> &generics)
    {
        Position position;

        if (this->scopes.top()->in_function())
        {
            position = Position::save(this->scopes.top()->builder());
        }

        auto scope = Scope::create(generic->scope);
        this->scopes.push(scope);

        for (size_t i = 0; i < generic->generics.size(); i++)
        {
            auto &name = generic->generics[i]->name;
            scope->add_name(name, generics[i]);
        }

        auto function = this->visitFunction(generic);

        this->scopes.pop();

        position.load(this->scopes.top()->builder());

        return function;
    }

    Namespace *visitNamespaceStatement(SanParser::NamespaceStatementContext *context)
    {
        auto scope = this->scopes.top();

        auto attributes = this->visitAttributes(context->attributes());

        if (!attributes.accept_current_target())
            return nullptr;

        auto name = context->VariableName()->getText();

        auto names = scope->get_names(name);

        if (!names->empty())
        {
            if (auto nsp = dynamic_cast<Namespace *>(names->last()))
            {
                this->scopes.push(nsp->scope);

                this->visitStatements(context->statement());

                this->scopes.pop();

                return nsp;
            }
        }

        auto nsp_scope = Scope::create(scope);
        auto nsp = new Namespace(name, nsp_scope);

        scope->add_name(name, nsp);

        this->scopes.push(nsp_scope);

        this->visitStatements(context->statement());

        this->scopes.pop();

        return nsp;
    }

    Block *visitBody(SanParser::BodyContext *context, Values::Function *function = nullptr)
    {
        auto scope = this->scopes.create();

        auto block = Block::create(scope->builder(), "entry");

        if (function == nullptr)
        {
            block->br(scope->builder());
            scope->get_function()->insert(block);
        }
        else
        {
            function->insert(block);
        }

        block->insert_point(scope->builder());

        if (function != nullptr)
        {
            function->entry_block = block;
            function->return_block = Block::create(scope->builder(), "return_label");

            auto function_type = function->get_type();
            auto return_type = function_type->return_type;

            auto function_ref = function->get_ref();

            if (!return_type->is_void())
            {
                if (function_type->is_sret)
                {
                    llvm::Argument *return_arg = function_ref->arg_begin();
                    function->return_value = new Values::Variable("retval", return_type, return_arg);
                }
                else
                {
                    auto alloca = scope->builder().CreateAlloca(return_type->get_ref(), nullptr, "retval");
                    function->return_value = new Values::Variable("retval", return_type, alloca);
                }
            }

            auto it = function_ref->arg_begin();

            if (function_type->is_sret)
            {
                it->setName("return_value");
                it++;
            }

            auto fa = function_type->args.begin();

            while (it != function_ref->arg_end())
            {
                it->setName(fa->name);

                llvm::AllocaInst *addr = this->env.builder.CreateAlloca(it->getType(), nullptr, fa->name + ".addr");
                this->env.builder.CreateStore(llvm::cast<llvm::Value>(it), addr, false);

                scope->add_name(fa->name, new Values::Variable(fa->name, fa->type, llvm::cast<llvm::Value>(addr)));

                it++;
                fa++;
            }

            if (!function_type->is_sret && function->return_value != nullptr && !dynamic_cast<Types::ClassType *>(function_type->return_type))
            {
                auto allocated_type = llvm::cast<llvm::AllocaInst>(function->return_value->get_ref())->getAllocatedType();
                auto type = new Type("", allocated_type);

                scope->builder().CreateStore(type->default_value(), function->return_value->get_ref());
            }
        }

        block->status = this->visitStatements(context->statement());

        if (block->status == StatementStatus::Breaked && scope->in_loop())
        {
            scope->get_loop()->end_label->br(scope->builder());
        }

        if (block->status != StatementStatus::Returned)
        {
            this->scopes.call_destructors(scope);
        }

        if (function != nullptr)
        {
            if (block->status != StatementStatus::Returned)
            {
                function->return_block->br(scope->builder());
            }

            function->insert(function->return_block);
            function->return_block->insert_point(scope->builder());

            auto function_type = function->get_type();
            auto return_type = function_type->return_type;

            if (function_type->is_sret || return_type->is_void())
            {
                scope->builder().CreateRetVoid();
            }
            else
            {
                const auto return_value = scope->builder().CreateLoad(function->return_value->get_ref());
                scope->builder().CreateRet(return_value);
            }
        }

        this->scopes.pop_no_destruct();

        return block;
    }

    Values::Variable *visitVariableDeclaration(SanParser::VariableDeclarationContext *context)
    {
        auto scope = this->scopes.top();

        auto name = context->VariableName()->getText();

        Type *type = nullptr;
        Value *rvalue = nullptr;

        if (auto type_context = context->type())
        {
            type = this->visitType(type_context);
        }

        if (auto expression = context->expression())
        {
            rvalue = this->valueFromExpression(expression);

            if (type == nullptr)
            {
                type = rvalue->type;

                if (type->is_function() && !type->is_pointer())
                {
                    type = type->pointer();
                }
            }
        }

        if (scope->in_function())
        {
            if (auto variable = dynamic_cast<Values::Variable *>(rvalue))
            {
                if (variable->can_be_taken && variable->type->equals(type))
                {
                    variable->get_ref()->setName(name);
                    scope->add_name(name, variable);

                    return variable;
                }
            }

            auto var = Values::Variable::create(name, type, scope->builder());

            // if (rexpr->can_be_taken)
            // {
            //     rexpr->copy(var, scope->builder, this->env.module);
            // }
            // else
            if (rvalue != nullptr)
            {
                var->store(rvalue, scope->builder(), scope->module(), true);
            }

            scope->add_name(name, var);

            return var;
        }
        else
        {
            if (rvalue == nullptr)
            {
                rvalue = Values::Constant::null_value(type);
            }

            if (auto constant = dynamic_cast<Values::Constant *>(rvalue))
            {
                auto casted_constant = constant->cast(type, scope->builder());
                auto global = Values::GlobalVariable::create(name, scope->module(), type, casted_constant);
                scope->add_name(name, global);

                return global;
            }
            else
            {
                throw InvalidRightValueException(context->expression()->getStart());
            }
        }

        return nullptr;
    }

    void visitReturnStatement(SanParser::ReturnStatementContext *context)
    {
        auto scope = this->scopes.top();
        auto function = scope->get_function();

        if (auto expression_context = context->expression())
        {
            auto rvalue = this->valueFromExpression(expression_context);
            function->return_value->store(rvalue, scope->builder(), scope->module(), true);
        }

        this->scopes.call_destructors(scope);
        function->return_block->br(scope->builder());
    }

    void visitIfStatement(SanParser::IfStatementContext *context)
    {
        auto scope = this->scopes.create();

        auto if_then = Block::create(scope->builder(), "if.then");
        auto if_end = Block::create(scope->builder(), "if.end");
        auto if_next = if_end;

        if (context->elseStatement())
        {
            if_next = Block::create(scope->builder(), "if.else");
        }

        if (auto expression = context->expression())
        {
            Value *value = this->valueFromExpression(expression);

            if (!value->type->is_boolean())
            {
                value = value->load(scope->builder())->not_equal(scope->builder(), Values::Constant::null_value(value->type));
            }

            if_then->conditional_br(scope->builder(), value->load_alloca_and_reference(scope->builder()), if_next);
        }
        else if (auto variable_declaration = context->variableDeclaration())
        {
            Value *value = this->visitVariableDeclaration(variable_declaration);

            if (!value->type->is_boolean())
            {
                value = value->load(scope->builder())->not_equal(scope->builder(), Values::Constant::null_value(value->type));
            }

            if_then->conditional_br(scope->builder(), value->load_alloca_and_reference(scope->builder()), if_next);
        }

        scope->get_function()->insert(if_then);
        if_then->insert_point(scope->builder());

        if_then->status = this->visitStatements({context->statement()});

        this->scopes.pop();

        if (if_then->status == StatementStatus::None)
        {
            if_end->br(scope->builder());
        }

        scope = this->scopes.top();

        scope->get_function()->insert(if_next);
        if_next->insert_point(scope->builder());

        if (auto else_statement = context->elseStatement())
        {
            if_next->status = this->visitElseStatement(else_statement);

            if (if_next->status == StatementStatus::None)
            {
                if_end->br(scope->builder());
            }

            scope->get_function()->insert(if_end);
            if_end->insert_point(scope->builder());
        }
    }

    StatementStatus visitElseStatement(SanParser::ElseStatementContext *context)
    {
        return this->visitStatements({context->statement()});
    }

    void visitWhileStatement(SanParser::WhileStatementContext *context)
    {
        auto scope = this->scopes.create();

        auto while_cond = Block::create(scope->builder(), "while.cond");
        auto while_body = Block::create(scope->builder(), "while.body");
        auto while_end = Block::create(scope->builder(), "while.end");

        auto loop = new Loop(while_end);
        scope->set_loop(loop);

        while_cond->br(scope->builder());

        scope->get_function()->insert(while_cond);
        while_cond->insert_point(scope->builder());

        auto value = this->valueFromExpression(context->expression());
        if (!value->type->is_boolean())
        {
            value = value->load_alloca_and_reference(scope->builder())->not_equal(scope->builder(), Values::Constant::null_value(value->type));
        }

        while_body->conditional_br(scope->builder(), value, while_end);

        scope->get_function()->insert(while_body);
        while_body->insert_point(scope->builder());

        while_body->status = this->visitStatements({context->statement()});

        if (while_body->status == StatementStatus::Breaked)
        {
            while_end->br(scope->builder());
        }
        else if (while_body->status != StatementStatus::Returned)
        {
            while_cond->br(scope->builder());
        }

        scope->get_function()->insert(while_end);
        while_end->insert_point(scope->builder());

        this->scopes.pop();
    }

    void visitForStatement(SanParser::ForStatementContext *context)
    {
        auto scope = this->scopes.create();

        auto for_cond = Block::create(scope->builder(), "for.cond");
        auto for_body = Block::create(scope->builder(), "for.body");
        auto for_end = Block::create(scope->builder(), "for.end");

        auto loop = new Loop(for_end);
        scope->set_loop(loop);

        auto value = this->valueFromExpression(context->expression());
        auto type = dynamic_cast<Types::ClassType *>(value->type);

        if (type)
        {
            auto begin_functions = type->get_names("begin", value, scope->builder(), scope->module());
            auto end_functions = type->get_names("end", value, scope->builder(), scope->module());

            if (begin_functions->size() > 0 && end_functions->size() > 0)
            {
                auto begin = dynamic_cast<Values::Function *>(begin_functions->last());
                auto end = dynamic_cast<Values::Function *>(end_functions->last());

                begin->calling_variable = value;
                end->calling_variable = value;

                if (begin != nullptr && end != nullptr)
                {
                    auto begin_value = begin->call(scope->builder(), scope->module())->load_alloca_and_reference(scope->builder());

                    auto iterator_name = context->VariableName()->getText();
                    auto iterator = Values::Variable::create(iterator_name, begin_value->type, scope->builder());
                    iterator->store(begin_value, scope->builder(), scope->module(), true);
                    scope->add_name(iterator_name, iterator);

                    for_cond->br(scope->builder());

                    scope->get_function()->insert(for_cond);
                    for_cond->insert_point(scope->builder());

                    auto end_value = end->call(scope->builder(), scope->module())->load_alloca_and_reference(scope->builder());

                    auto condition = iterator->load_alloca_and_reference(scope->builder())->not_equal(scope->builder(), end_value);
                    for_body->conditional_br(scope->builder(), condition, for_end);

                    scope->get_function()->insert(for_body);
                    for_body->insert_point(scope->builder());

                    for_body->status = this->visitStatements({context->statement()});

                    if (for_body->status == StatementStatus::Breaked)
                    {
                        for_end->br(scope->builder());
                    }
                    else if (for_body->status != StatementStatus::Returned)
                    {
                        // Temporary before operator overloads
                        auto type = Type::i32(scope->context());
                        auto value = llvm::ConstantInt::get(type->get_ref(), 1, true);
                        auto constant = new Values::Constant("literal_i32", type, value);

                        iterator->add(scope->module(), scope->builder(), constant);

                        for_cond->br(scope->builder());
                    }

                    scope->get_function()->insert(for_end);
                    for_end->insert_point(scope->builder());

                    this->scopes.pop();

                    return;
                }
            }
        }

        this->scopes.pop();

        throw InvalidRangeException(context->expression()->getStart());
    }

    void visitImportStatement(SanParser::ImportStatementContext *context)
    {
        auto str = this->stringLiteralToString(context->StringLiteral()->getText());

        try
        {
            this->from_file(str);
        }
        catch (FileNotFoundException e)
        {
            throw ImportException(context->StringLiteral()->getSymbol());
        }
    }

    Types::ClassType *visitSpecialClassStatement(SanParser::SpecialClassStatementContext *context)
    {
        auto scope = this->scopes.top();

        auto attributes = this->visitAttributes(context->attributes());

        if (!attributes.accept_current_target())
            return nullptr;

        auto class_scope = Scope::create(scope);

        auto scoped_name_context = context->scopedNameNoGeneric();
        auto names = this->visitScopedNameNoGeneric(scoped_name_context);
        auto classes = names->get_generic_classes();

        if (!classes->empty())
        {
            // Temporary take the first generic
            if (auto base = dynamic_cast<Types::GenericClassType *>(classes->last()))
            {
                Position position;

                if (this->scopes.top()->in_function())
                {
                    position = Position::save(this->scopes.top()->builder());
                }

                this->scopes.push(class_scope);

                class_scope->add_name("base", base);

                auto generics = this->visitClassTypeNameGenerics(context->classTypeNameGenerics());

                auto type = Types::ClassType::create(class_scope, base->name, generics);

                if (auto extends = context->classExtends())
                {
                    type->parents = this->visitClassExtends(extends);
                }

                base->children.push_back(type);

                this->visitClassBody(context->classBody(), type->parents, type);

                this->scopes.pop();

                position.load(this->scopes.top()->builder());

                return type;
            }
        }

        throw NotAGenericException(scoped_name_context->getStart());
    }

    Name *visitUnionStatement(SanParser::UnionStatementContext *context)
    {
        auto scope = this->scopes.top();

        auto attributes = this->visitAttributes(context->attributes());

        if (!attributes.accept_current_target())
            return nullptr;

        auto name = context->VariableName()->getText();
        auto union_scope = Scope::create(scope);

        auto type = Types::UnionType::create(union_scope, name);
        scope->add_name(name, type);

        this->scopes.push(union_scope);

        this->visitUnionBody(context->unionBody(), type);

        this->scopes.pop();

        return type;
    }

    Types::UnionType *visitUnionBody(SanParser::UnionBodyContext *context, Types::UnionType *type)
    {
        auto scope = this->scopes.top();

        std::vector<Types::UnionProperty *> properties;

        for (auto &union_property : context->unionProperty())
        {
            auto property = this->visitUnionProperty(union_property);
            properties.push_back(property);
        }

        type->set_properties(properties, scope->builder(), scope->module());

        return type;
    }

    Types::UnionProperty *visitUnionProperty(SanParser::UnionPropertyContext *context)
    {
        auto name = context->VariableName()->getText();
        auto type = this->visitType(context->type());

        return new Types::UnionProperty(name, type);
    }

    Name *visitClassStatement(SanParser::ClassStatementContext *context)
    {
        auto scope = this->scopes.top();

        auto attributes = this->visitAttributes(context->attributes());

        if (!attributes.accept_current_target())
            return nullptr;

        auto class_scope = Scope::create(scope);

        auto name = context->VariableName()->getText();

        if (auto generics_context = context->classGenerics())
        {
            auto generics = this->visitClassGenerics(generics_context);
            auto type = new Types::GenericClassType(class_scope, name, generics, context);

            scope->add_name(name, type);

            return type;
        }
        else
        {
            auto type = Types::ClassType::create(class_scope, name);
            scope->add_name(name, type);

            if (auto extends = context->classExtends())
            {
                type->parents = this->visitClassExtends(extends);
            }

            this->scopes.push(class_scope);

            this->visitClassBody(context->classBody(), type->parents, type);

            this->scopes.pop();

            return type;
        }
    }

    Types::ClassType *generateGenericClassType(Types::GenericClassType *generic, const std::vector<Type *> &generics)
    {
        Position position;

        if (this->scopes.top()->in_function())
        {
            position = Position::save(this->scopes.top()->builder());
        }

        auto scope = Scope::create(generic->scope);
        this->scopes.push(scope);

        auto type = Types::ClassType::create(scope, generic->name, generics);
        generic->children.push_back(type);

        for (size_t i = 0; i < generic->generics.size(); i++)
        {
            auto &name = generic->generics[i]->name;
            scope->add_name(name, generics[i]);
        }

        if (auto extends = generic->context->classExtends())
        {
            type->parents = this->visitClassExtends(extends);
        }

        this->visitClassBody(generic->context->classBody(), type->parents, type);

        this->scopes.pop();

        position.load(this->scopes.top()->builder());

        return type;
    }

    std::vector<Types::Generic *> visitClassGenerics(SanParser::ClassGenericsContext *context)
    {
        std::vector<Types::Generic *> generics;

        for (const auto &name : context->VariableName())
        {
            auto generic = new Types::Generic(name->getText(), nullptr);
            generics.push_back(generic);
        }

        return generics;
    }

    std::vector<Types::ClassType *> visitClassExtends(SanParser::ClassExtendsContext *context)
    {
        auto scope = this->scopes.top();
        std::vector<Types::ClassType *> types;

        for (auto &type_name : context->classTypeName())
        {
            auto type = this->visitClassTypeName(type_name);
            types.push_back(type);
        }

        return types;
    }

    Types::ClassType *visitClassBody(SanParser::ClassBodyContext *context, std::vector<Types::ClassType *> parents, Types::ClassType *type)
    {
        auto scope = this->scopes.top();

        auto generate_methods = generating_properties_stack == 0;
        auto struct_type = type->get_ref();

        std::vector<llvm::Type *> properties_types;

        for (auto &parent : parents)
        {
            properties_types.push_back(parent->ref);
        }

        this->generating_properties_stack++;
        for (auto &class_property : context->classProperty())
        {
            auto property = this->visitClassProperty(class_property);

            if (!class_property->Static())
            {
                type->properties.push_back(property);
                properties_types.push_back(property->type->get_ref());
            }
            else
            {
                auto variable = Values::GlobalVariable::create(property->name, this->env.module, property->type, property->default_value);
                type->static_scope->add_name(property->name, variable);
            }
        }

        struct_type->setBody(properties_types, true);
        this->generating_properties_stack--;

        auto class_methods = context->classMethod();
        type->pending_methods = class_methods;

        if (generate_methods)
        {
            this->generatePendingMethods(type);
        }

        return type;
    }

    std::vector<Values::Function *> generatePendingMethods(Types::ClassType *type)
    {
        // Extract and clear pending_methods to prevent recursive generation
        auto pending_methods = type->pending_methods;
        type->pending_methods.clear();

        std::vector<Values::Function *> methods;

        for (auto &generic : type->generics)
        {
            if (auto class_type = dynamic_cast<Types::ClassType *>(generic->get_base()))
            {
                if (!class_type->pending_methods.empty())
                {
                    this->scopes.push(class_type->static_scope);
                    this->generatePendingMethods(class_type);
                    this->scopes.pop();
                }
            }
        }

        for (auto &class_method : pending_methods)
        {
            auto is_static = !!class_method->Static();

            auto method = this->generateClassMethodDeclaration(class_method, type, is_static);

            if (auto function = dynamic_cast<Values::Function *>(method))
            {
                methods.push_back(function);
            }

            if (is_static)
            {
                type->static_scope->add_name(method->name, method);
            }
            else
            {
                type->scope->add_name(method->name, method);
            }
        }

        for (auto property : type->properties)
        {
            if (auto class_type = dynamic_cast<Types::ClassType *>(property->type->get_base()))
            {
                if (!class_type->pending_methods.empty())
                {
                    this->scopes.push(class_type->static_scope);
                    this->generatePendingMethods(class_type);
                    this->scopes.pop();
                }
            }
        }

        for (size_t i = 0; i < methods.size(); i++)
        {
            auto method = methods[i];

            // if (!method->is_base)
            {
                auto class_method = pending_methods[i];

                this->generateClassMethodBody(class_method, method);
            }
        }

        return methods;
    }

    Types::ClassProperty *visitClassProperty(SanParser::ClassPropertyContext *context)
    {
        auto name = context->VariableName()->getText();
        auto type = this->visitType(context->type());

        Values::Constant *default_value = nullptr;

        if (auto expression_context = context->expression())
        {
            auto value = this->valueFromExpression(context->expression());

            if (auto constant = dynamic_cast<Values::Constant *>(value))
            {
                default_value = constant;
            }
            else
            {
                throw InvalidRightValueException(expression_context->getStart());
            }
        }

        return new Types::ClassProperty(name, type, default_value);
    }

    /**
     * Return value can be a pointer of Function or GenericFunctionType
     */
    Name *generateClassMethodDeclaration(SanParser::ClassMethodContext *context, Types::ClassType *parent, const bool &is_static)
    {
        if (is_static)
        {
            return this->visitFunction(context->function(), false, false);
        }
        else
        {
            return this->visitFunction(context->function(), false, false, parent);
        }
    }

    Values::Function *generateClassMethodBody(SanParser::ClassMethodContext *context, Values::Function *base)
    {
        return this->generateFunctionBody(context->function(), base);
    }

    Name *visitExpression(SanParser::ExpressionContext *context)
    {
        if (const auto in_paren_expression_context = dynamic_cast<SanParser::InParenExpressionContext *>(context))
        {
            return this->visitInParenExpression(in_paren_expression_context);
        }
        else if (const auto sizeof_expression_context = dynamic_cast<SanParser::SizeofExpressionContext *>(context))
        {
            return this->visitSizeofExpression(sizeof_expression_context);
        }
        else if (const auto class_instantiation_expression_context = dynamic_cast<SanParser::ClassInstantiationExpressionContext *>(context))
        {
            return this->visitClassInstantiationExpression(class_instantiation_expression_context);
        }
        else if (const auto function_call_expression_context = dynamic_cast<SanParser::FunctionCallExpressionContext *>(context))
        {
            return this->visitFunctionCallExpression(function_call_expression_context);
        }
        else if (const auto binary_operation_context = dynamic_cast<SanParser::BinaryOperationContext *>(context))
        {
            return this->visitBinaryOperation(binary_operation_context);
        }
        else if (const auto binary_multiplicative_operation_context = dynamic_cast<SanParser::BinaryMultiplicativeOperationContext *>(context))
        {
            return this->visitBinaryMultiplicativeOperation(binary_multiplicative_operation_context);
        }
        else if (const auto binary_bitwise_operation_context = dynamic_cast<SanParser::BinaryBitwiseOperationContext *>(context))
        {
            return this->visitBinaryBitwiseOperation(binary_bitwise_operation_context);
        }
        else if (const auto binary_comparison_operation_context = dynamic_cast<SanParser::BinaryComparisonOperationContext *>(context))
        {
            return this->visitBinaryComparisonOperation(binary_comparison_operation_context);
        }
        else if (const auto binary_conditional_operation_context = dynamic_cast<SanParser::BinaryConditionalOperationContext *>(context))
        {
            return this->visitBinaryConditionalOperation(binary_conditional_operation_context);
        }
        else if (const auto equality_operation_context = dynamic_cast<SanParser::EqualityOperationContext *>(context))
        {
            return this->visitEqualityOperation(equality_operation_context);
        }
        else if (const auto index_context = dynamic_cast<SanParser::IndexContext *>(context))
        {
            return this->visitIndex(index_context);
        }
        else if (const auto type_cast_context = dynamic_cast<SanParser::TypeCastContext *>(context))
        {
            return this->visitTypeCast(type_cast_context);
        }
        else if (const auto property_expression_context = dynamic_cast<SanParser::PropertyExpressionContext *>(context))
        {
            return this->visitPropertyExpression(property_expression_context);
        }
        else if (const auto function_expression_context = dynamic_cast<SanParser::FunctionExpressionContext *>(context))
        {
            return this->visitFunction(function_expression_context->function());
        }
        else if (const auto name_expression_context = dynamic_cast<SanParser::NameExpressionContext *>(context))
        {
            return this->visitNameExpression(name_expression_context);
        }
        else if (const auto literal_declaration_context = dynamic_cast<SanParser::LiteralDeclarationContext *>(context))
        {
            return this->visitLiteralDeclaration(literal_declaration_context);
        }

        return nullptr;
    }

    Name *visitInParenExpression(SanParser::InParenExpressionContext *context)
    {
        return this->visitExpression(context->expression());
    }

    Values::Constant *visitSizeofExpression(SanParser::SizeofExpressionContext *context)
    {
        auto scope = this->scopes.top();

        Type *type = nullptr;

        if (auto expression_context = context->expression())
        {
            auto expression = this->visitExpression(expression_context);

            if (auto names = dynamic_cast<NameArray *>(expression))
            {
                auto name = names->last();

                while (auto alias = dynamic_cast<Alias *>(name))
                {
                    name = alias->names->last();
                }

                if (auto type_name = dynamic_cast<Type *>(name))
                {
                    type = type_name;
                }
                else if (auto variable = dynamic_cast<Values::Variable *>(name))
                {
                    type = variable->type;
                }
                else
                {
                    throw InvalidRightValueException(expression_context->getStart());
                }
            }
            else if (auto value = dynamic_cast<Value *>(expression))
            {
                type = value->type;
            }
            else
            {
                throw InvalidRightValueException(expression_context->getStart());
            }
        }
        else if (auto type_context = context->type())
        {
            type = this->visitType(type_context);
        }

        auto i64 = Type::i64(scope->context());
        auto value = llvm::ConstantInt::get(i64->get_ref(), type->size(this->env.module));

        return new Values::Constant("sizeof", i64, value);
    }

    Value *visitClassInstantiationExpression(SanParser::ClassInstantiationExpressionContext *context)
    {
        auto scope = this->scopes.top();
        auto type = this->visitClassTypeName(context->classTypeName());

        auto var = Values::Variable::create(type->name + ".inst", type, scope->builder());

        std::vector<std::string> assigned_properties;

        if (auto properties = context->classInstantiationProperties())
        {
            assigned_properties = this->visitClassInstantiationProperties(properties, var);
        }

        auto class_properties = type->get_all_properties(scope->module());
        for (auto &class_property : class_properties)
        {
            if (class_property->property->default_value == nullptr)
            {
                continue;
            }

            auto is_defined = std::find(assigned_properties.begin(), assigned_properties.end(), class_property->property->name) != assigned_properties.end();

            if (!is_defined)
            {
                Value *container = var;

                if (class_property->from != nullptr)
                {
                    container = container->struct_cast(class_property->from, class_property->padding, scope->builder());
                }

                auto ptr = container->struct_gep(class_property->property->name, class_property->property->type, class_property->index, scope->builder());

                ptr->store(class_property->property->default_value, scope->builder(), scope->module(), true);
            }
        }

        return var;
    }

    std::vector<std::string> visitClassInstantiationProperties(SanParser::ClassInstantiationPropertiesContext *context, Values::Variable *var)
    {
        auto scope = this->scopes.top();
        auto type = static_cast<Types::ClassType *>(var->type);

        std::vector<std::string> assigned_properties;

        for (auto &property : context->classInstantiationProperty())
        {
            this->visitClassInstantiationProperty(property, var);

            auto name = property->VariableName()->getText();
            assigned_properties.push_back(name);
        }

        return assigned_properties;
    }

    void visitClassInstantiationProperty(SanParser::ClassInstantiationPropertyContext *context, Values::Variable *var)
    {
        auto scope = this->scopes.top();
        auto type = static_cast<Types::ClassType *>(var->type);

        auto name = context->VariableName()->getText();

        Value *value = nullptr;

        if (auto expression = context->expression())
        {
            value = this->valueFromExpression(expression);
        }
        else
        {
            auto named_value = scope->get_names(name);
            value = this->valueFromName(named_value, context);
        }

        auto property_position = type->get_property(name, this->env.module);

        if (property_position == nullptr)
        {
            throw PropertyNotFoundException(context->VariableName()->getSymbol(), type);
        }

        Value *container = var;

        if (property_position->from != nullptr)
        {
            container = container->struct_cast(property_position->from, property_position->padding, scope->builder());
        }

        auto ptr = container->struct_gep(property_position->property->name, property_position->property->type, property_position->index, scope->builder());

        ptr->store(value, scope->builder(), scope->module(), true);
    }

    Value *visitFunctionCallExpression(SanParser::FunctionCallExpressionContext *context)
    {
        auto scope = this->scopes.top();
        auto lvalue = this->visitExpression(context->expression());
        auto args = this->visitFunctionCallArguments(context->functionCallArguments());

        if (auto value = dynamic_cast<Value *>(lvalue))
        {
            value = value->load_alloca_and_reference(scope->builder());

            if (auto type = dynamic_cast<Types::FunctionType *>(value->type))
            {
                if (type->compare_args(args))
                {
                    auto return_value = value->call(scope->builder(), scope->module(), args);
                    scope->add_name("", return_value);

                    return return_value;
                }
            }
        }
        else if (auto names = dynamic_cast<NameArray *>(lvalue))
        {
            while (auto alias = dynamic_cast<Alias *>(names->last()))
            {
                names = alias->names;
            }

            if (auto function = names->get_function(args))
            {
                if (auto value = dynamic_cast<Value *>(function))
                {
                    value = value->load_alloca_and_reference(scope->builder());

                    auto return_value = value->call(scope->builder(), scope->module(), args);
                    scope->add_name("", return_value);

                    return return_value;
                }
            }
        }

        throw NoFunctionMatchException(context->expression()->getStart(), args);
    }

    std::vector<Value *> visitFunctionCallArguments(SanParser::FunctionCallArgumentsContext *context)
    {
        std::vector<Value *> args;

        if (context == nullptr)
        {
            return args;
        }

        auto scope = this->scopes.top();

        for (auto arg : context->functionCallArgument())
        {
            auto var = this->visitFunctionCallArgument(arg);
            args.push_back(var);
        }

        return args;
    }

    Value *visitFunctionCallArgument(SanParser::FunctionCallArgumentContext *context)
    {
        return this->valueFromExpression(context->expression());
    }

    Value *visitBinaryOperation(SanParser::BinaryOperationContext *context)
    {
        auto scope = this->scopes.top();

        const auto opt = context->operatorStatement();
        const auto lexpr_context = context->expression(0);
        const auto rexpr_context = context->expression(1);

        auto lexpr = this->valueFromExpression(lexpr_context);
        auto rexpr = this->valueFromExpression(rexpr_context);

        auto lvalue = lexpr;
        auto rvalue = rexpr;

        if (opt->Add())
        {
            if (auto value = Value::add(scope->builder(), lvalue, rvalue))
            {
                return value;
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("+", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->Sub())
        {
            if (auto value = Value::sub(scope->builder(), lvalue, rvalue))
            {
                return value;
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("-", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }

        throw InvalidRightValueException(rexpr_context->getStart());
    }

    Value *visitBinaryMultiplicativeOperation(SanParser::BinaryMultiplicativeOperationContext *context)
    {
        auto scope = this->scopes.top();

        const auto opt = context->multiplicativeOperatorStatement();
        const auto lexpr_context = context->expression(0);
        const auto rexpr_context = context->expression(1);

        auto lexpr = this->valueFromExpression(lexpr_context);
        auto rexpr = this->valueFromExpression(rexpr_context);

        auto lvalue = lexpr->load_alloca_and_reference(scope->builder());
        auto rvalue = rexpr->cast(lvalue->type, scope->builder());

        if (opt->Mul())
        {
            if (lvalue->type->is_integer())
            {
                auto value = this->env.builder.CreateNSWMul(lvalue->get_ref(), rvalue->get_ref());
                return new Value("mul", lvalue->type, value);
            }
            else if (lvalue->type->is_floating_point())
            {
                auto value = this->env.builder.CreateFMul(lvalue->get_ref(), rvalue->get_ref());
                return new Value("mul", lvalue->type, value);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("*", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->Div())
        {
            if (lvalue->type->is_integer())
            {
                auto value = this->env.builder.CreateSDiv(lvalue->get_ref(), rvalue->get_ref());
                return new Value("div", lvalue->type, value);
            }
            else if (lvalue->type->is_floating_point())
            {
                auto value = this->env.builder.CreateFDiv(lvalue->get_ref(), rvalue->get_ref());
                return new Value("div", lvalue->type, value);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("/", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->Mod())
        {
            if (lvalue->type->is_integer())
            {
                auto value = this->env.builder.CreateSRem(lvalue->get_ref(), rvalue->get_ref());
                return new Value("mod", lvalue->type, value);
            }
            else if (lvalue->type->is_floating_point())
            {
                auto value = this->env.builder.CreateFRem(lvalue->get_ref(), rvalue->get_ref());
                return new Value("mod", lvalue->type, value);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("%", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }

        return nullptr;
    }

    Value *visitBinaryBitwiseOperation(SanParser::BinaryBitwiseOperationContext *context)
    {
        auto scope = this->scopes.top();

        const auto opt = context->bitwiseOperatorStatement();
        const auto lexpr_context = context->expression(0);
        const auto rexpr_context = context->expression(1);

        auto lexpr = this->valueFromExpression(lexpr_context);
        auto rexpr = this->valueFromExpression(rexpr_context);

        auto lvalue = lexpr->load_alloca_and_reference(scope->builder());
        auto rvalue = rexpr->cast(lvalue->type, scope->builder());

        if (opt->Xor())
        {
            if (lvalue->type->is_integer())
            {
                auto value = this->env.builder.CreateXor(lvalue->get_ref(), rvalue->get_ref());
                return new Value("xor", lvalue->type, value);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("^", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->BitwiseOr())
        {
            if (lvalue->type->is_integer())
            {
                auto value = this->env.builder.CreateOr(lvalue->get_ref(), rvalue->get_ref());
                return new Value("or", lvalue->type, value);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("|", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->BitwiseAnd())
        {
            if (lvalue->type->is_integer())
            {
                auto value = this->env.builder.CreateAnd(lvalue->get_ref(), rvalue->get_ref());
                return new Value("and", lvalue->type, value);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("&", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }

        return nullptr;
    }

    Value *visitBinaryComparisonOperation(SanParser::BinaryComparisonOperationContext *context)
    {
        auto scope = this->scopes.top();

        const auto opt = context->comparisonOperatorStatement();
        const auto lexpr_context = context->expression(0);
        const auto rexpr_context = context->expression(1);

        auto lexpr = this->valueFromExpression(lexpr_context);
        auto rexpr = this->valueFromExpression(rexpr_context);

        auto lvalue = lexpr->load_alloca_and_reference(scope->builder());
        auto rvalue = rexpr->cast(lvalue->type, scope->builder());

        if (opt->EqualTo())
        {
            if (lvalue->type->is_integer() || lvalue->type->is_pointer())
            {
                auto value = this->env.builder.CreateICmpEQ(lvalue->get_ref(), rvalue->get_ref());
                return new Value("eq", Type::i1(scope->context()), value, false);
            }
            else if (lvalue->type->is_floating_point())
            {
                auto value = this->env.builder.CreateFCmpOEQ(lvalue->get_ref(), rvalue->get_ref());
                return new Value("eq", Type::i1(scope->context()), value, false);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("==", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->NotEqualTo())
        {
            if (auto value = lvalue->not_equal(scope->builder(), rvalue))
            {
                return value;
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("!=", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->LessThan())
        {
            if (lvalue->type->is_integer() || lvalue->type->is_pointer())
            {
                auto value = this->env.builder.CreateICmpSLT(lvalue->get_ref(), rvalue->get_ref());
                return new Value("lt", Type::i1(scope->context()), value, false);
            }
            else if (lvalue->type->is_floating_point())
            {
                auto value = this->env.builder.CreateFCmpOLT(lvalue->get_ref(), rvalue->get_ref());
                return new Value("lt", Type::i1(scope->context()), value, false);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("<", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->LessThanOrEqualTo())
        {
            if (lvalue->type->is_integer() || lvalue->type->is_pointer())
            {
                auto value = this->env.builder.CreateICmpSLE(lvalue->get_ref(), rvalue->get_ref());
                return new Value("lte", Type::i1(scope->context()), value, false);
            }
            else if (lvalue->type->is_floating_point())
            {
                auto value = this->env.builder.CreateFCmpOLE(lvalue->get_ref(), rvalue->get_ref());
                return new Value("lte", Type::i1(scope->context()), value, false);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload("<=", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->GreaterThan())
        {
            if (lvalue->type->is_integer() || lvalue->type->is_pointer())
            {
                auto value = this->env.builder.CreateICmpSGT(lvalue->get_ref(), rvalue->get_ref());
                return new Value("gt", Type::i1(scope->context()), value, false);
            }
            else if (lvalue->type->is_floating_point())
            {
                auto value = this->env.builder.CreateFCmpOGT(lvalue->get_ref(), rvalue->get_ref());
                return new Value("gt", Type::i1(scope->context()), value, false);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload(">", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }
        else if (opt->GreaterThanOrEqualTo())
        {
            if (lvalue->type->is_integer() || lvalue->type->is_pointer())
            {
                auto value = this->env.builder.CreateICmpSGE(lvalue->get_ref(), rvalue->get_ref());
                return new Value("gte", Type::i1(scope->context()), value, false);
            }
            else if (lvalue->type->is_floating_point())
            {
                auto value = this->env.builder.CreateFCmpOGE(lvalue->get_ref(), rvalue->get_ref());
                return new Value("gte", Type::i1(scope->context()), value, false);
            }

            auto args = {lexpr, rexpr};
            if (auto overload = this->getOperatorOverload(">=", args))
            {
                return overload->call(scope->builder(), scope->module(), args);
            }
        }

        return nullptr;
    }

    Value *visitBinaryConditionalOperation(SanParser::BinaryConditionalOperationContext *context)
    {
        auto scope = this->scopes.top();

        auto opt = context->conditionalOperatorStatement();

        auto lexpr_context = context->expression(0);
        auto lexpr = this->valueFromExpression(lexpr_context);

        auto cond_false = Block::create(scope->builder());
        auto cond_end = Block::create(scope->builder());

        llvm::Constant *boolean_constant = nullptr;

        if (dynamic_cast<Values::Constant *>(lexpr))
        {
            auto variable = Values::Variable::create(lexpr->name, lexpr->type, scope->builder());
            variable->store(lexpr, scope->builder(), scope->module());

            lexpr = variable->load(scope->builder())->not_equal(scope->builder(), Values::Constant::null_value(lexpr->type));
        }

        if (opt->ConditionalOr())
        {
            boolean_constant = llvm::ConstantInt::getTrue(scope->context());

            cond_false->set_name("lor.rhs");
            cond_end->set_name("lor.end");

            cond_end->conditional_br(scope->builder(), lexpr, cond_false);
        }
        else if (opt->ConditionalAnd())
        {
            boolean_constant = llvm::ConstantInt::getFalse(scope->context());

            cond_false->set_name("land.rhs");
            cond_end->set_name("land.end");

            cond_false->conditional_br(scope->builder(), lexpr, cond_end);
        }

        scope->get_function()->insert(cond_false);
        cond_false->insert_point(scope->builder());

        auto rexpr_context = context->expression(1);
        auto rexpr = this->valueFromExpression(rexpr_context);

        if (dynamic_cast<Values::Constant *>(rexpr))
        {
            auto variable = Values::Variable::create(rexpr->name, rexpr->type, scope->builder());
            variable->store(rexpr, scope->builder(), scope->module());

            rexpr = variable->load(scope->builder())->not_equal(scope->builder(), Values::Constant::null_value(rexpr->type));
        }

        cond_end->br(scope->builder());

        scope->get_function()->insert(cond_end);
        cond_end->insert_point(scope->builder());

        auto phi = scope->builder().CreatePHI(Type::llvm_i1(scope->context()), 2);
        phi->addIncoming(boolean_constant, reinterpret_cast<llvm::Instruction *>(lexpr->get_ref())->getParent());
        phi->addIncoming(rexpr->get_ref(), reinterpret_cast<llvm::Instruction *>(rexpr->get_ref())->getParent());

        return new Value("phi", Type::i1(scope->context()), phi, false);
    }

    Value *visitEqualityOperation(SanParser::EqualityOperationContext *context)
    {
        auto scope = this->scopes.top();

        const auto opt = context->equalityOperatorStatement();
        const auto lexpr_context = context->expression(0);
        const auto rexpr_context = context->expression(1);

        auto lexpr = this->valueFromExpression(lexpr_context);
        auto rexpr = this->valueFromExpression(rexpr_context);

        auto args = {lexpr, rexpr};
        if (auto overload = this->getOperatorOverload("=", args))
        {
            return overload->call(scope->builder(), scope->module(), args);
        }
        else
        {
            auto rtype = rexpr->type;

            if (rtype->is_function() && !rtype->is_pointer())
            {
                rtype = rtype->pointer();
            }

            if (lexpr->type->compatibility(rtype) != Type::NOT_COMPATIBLE)
            {
                lexpr->store(rexpr, scope->builder(), scope->module());
                return lexpr;
            }
        }

        throw InvalidRightValueException(rexpr_context->getStart());
    }

    Value *getOperatorOverload(const std::string &name, const std::vector<Value *> &args)
    {
        auto scope = this->scopes.top();
        auto names = scope->get_names(name);

        if (auto match = names->get_function(args))
        {
            if (auto value = dynamic_cast<Value *>(match))
            {
                return value;
            }
        }

        return nullptr;
    }

    Value *getCastOverload(Value *value, Type *dest)
    {
        auto scope = this->scopes.top();
        auto names = scope->get_names("@cast");

        if (auto match = names->get_function({value}, dest))
        {
            if (auto value = dynamic_cast<Value *>(match))
            {
                return value;
            }
        }

        return nullptr;
    }

    Value *visitIndex(SanParser::IndexContext *context)
    {
        auto scope = this->scopes.top();

        auto expression = this->valueFromExpression(context->expression(0));
        auto index = this->valueFromExpression(context->expression(1));

        return expression->gep(index, scope->builder());
    }

    Value *visitTypeCast(SanParser::TypeCastContext *context)
    {
        auto scope = this->scopes.top();
        auto expr = this->valueFromExpression(context->expression());
        auto type = this->visitType(context->type());

        if (auto function = this->getCastOverload(expr, type))
        {
            return function->call(scope->builder(), scope->module(), {expr});
        }

        return expr->cast(type, scope->builder(), true);
    }

    Name *visitPropertyExpression(SanParser::PropertyExpressionContext *context)
    {
        auto scope = this->scopes.top();
        auto expr = this->valueFromExpression(context->expression());

        if (expr->is_alloca)
        {
            expr = expr->load_reference(scope->builder());
        }

        if (context->Arrow())
        {
            if (!expr->type->is_pointer())
            {
                throw NotAPointerException(context->expression()->getStart());
            }

            auto index = 0UL;
            expr = expr->load(scope->builder())->gep(index, scope->builder());
        }

        auto type = expr->type;

        if (type->is_reference)
        {
            type = type->base;
        }

        if (auto class_type = dynamic_cast<Types::ClassType *>(type))
        {
            return this->visitName(context->name(), expr);
        }
        else if (auto union_type = dynamic_cast<Types::UnionType *>(type))
        {
            return this->visitName(context->name(), expr);
        }

        throw NotAClassException(context->expression()->getStart());
    }

    NameArray *visitNameExpression(SanParser::NameExpressionContext *context)
    {
        return this->visitScopedName(context->scopedName());
    }

    Value *valueFromExpression(SanParser::ExpressionContext *context)
    {
        auto expression = this->visitExpression(context);
        return this->valueFromName(expression, context);
    }

    Value *valueFromName(Name *name, antlr4::ParserRuleContext *context)
    {
        if (auto array = dynamic_cast<NameArray *>(name))
        {
            while (auto alias = dynamic_cast<Alias *>(array->last()))
            {
                array = alias->names;
            }

            if (array->size() > 1 && !dynamic_cast<Values::Variable *>(array->get(0)))
            {
                throw MultipleInstancesException(context->getStart());
            }

            if (auto value = dynamic_cast<Value *>(array->last()))
            {
                return value;
            }
        }
        else if (auto value = dynamic_cast<Value *>(name))
        {
            return value;
        }

        throw InvalidValueException(context->getStart());
    }

    Type *typeFromName(Name *name, antlr4::ParserRuleContext *context)
    {
        if (auto array = dynamic_cast<NameArray *>(name))
        {
            while (auto alias = dynamic_cast<Alias *>(array->last()))
            {
                array = alias->names;
            }

            if (auto type = dynamic_cast<Type *>(array->last()))
            {
                return type;
            }
        }
        else if (auto type = dynamic_cast<Type *>(name))
        {
            return type;
        }

        throw InvalidTypeException(context->getStart());
    }

    NameArray *visitScopedName(SanParser::ScopedNameContext *context)
    {
        if (auto scope_resolver_context = context->scopeResolver())
        {
            auto scope = this->visitScopeResolver(scope_resolver_context);
            return this->visitName(context->name(), scope);
        }

        auto scope = this->scopes.top();
        return this->visitName(context->name(), scope);
    }

    NameArray *visitScopedNameNoGeneric(SanParser::ScopedNameNoGenericContext *context)
    {
        auto scope = this->scopes.top();

        if (auto scope_resolver_context = context->scopeResolver())
        {
            scope = this->visitScopeResolver(scope_resolver_context);
        }

        return this->visitNameNoGeneric(context->nameNoGeneric(), scope);
    }

    std::shared_ptr<Scope> visitScopeResolver(SanParser::ScopeResolverContext *context)
    {
        return this->visitScopeResolver(context, this->scopes.top());
    }

    std::shared_ptr<Scope> scopeFromName(Name *name)
    {
        while (auto alias = dynamic_cast<Alias *>(name))
        {
            name = alias->names->last();
        }

        if (auto class_type = dynamic_cast<Types::ClassType *>(name))
        {
            return class_type->get_static_scope();
        }
        else if (auto nsp = dynamic_cast<Namespace *>(name))
        {
            return nsp->scope;
        }

        return nullptr;
    }

    std::shared_ptr<Scope> visitScopeResolver(SanParser::ScopeResolverContext *context, std::shared_ptr<Scope> scope)
    {
        auto names = this->visitName(context->name(), scope);
        auto name = names->last();

        std::shared_ptr<Scope> resolved_scope = this->scopeFromName(name);

        if (resolved_scope != nullptr)
        {
            if (auto scope_resolver_context = context->scopeResolver())
            {
                return this->visitScopeResolver(scope_resolver_context, resolved_scope);
            }

            return resolved_scope;
        }

        throw NotAClassOrNamespaceException(context->name()->getStart());
    }

    NameArray *visitName(SanParser::NameContext *context, std::shared_ptr<Scope> &scope)
    {
        auto name = context->VariableName()->getText();
        auto names = scope->get_names(name);

        if (!names->empty())
        {
            if (auto generics_context = context->classTypeNameGenerics())
            {
                return this->visitTypeNameClassGenerics(generics_context, names);
            }

            return names;
        }

        throw UnknownNameException(context->VariableName()->getSymbol());
    }

    NameArray *visitName(SanParser::NameContext *context, Value *value)
    {
        auto scope = this->scopes.top();

        if (auto type = dynamic_cast<Types::ClassType *>(value->type->behind_reference()))
        {
            auto name = context->VariableName()->getText();
            auto names = type->get_names(name, value, scope->builder(), scope->module());

            if (!names->empty())
            {
                if (auto generics_context = context->classTypeNameGenerics())
                {
                    names = this->visitTypeNameClassGenerics(generics_context, names);
                }

                for (auto &name : names->names)
                {
                    if (auto name_value = dynamic_cast<Value *>(name))
                    {
                        name_value->calling_variable = value;
                    }
                }

                return names;
            }

            throw UnknownNameException(context->VariableName()->getSymbol());
        }
        else if (auto type = dynamic_cast<Types::UnionType *>(value->type->behind_reference()))
        {
            auto name = context->VariableName()->getText();
            auto property = type->get_property(name);

            if (property != nullptr)
            {
                return new NameArray({value->union_cast(property->type, scope->builder())});
            }

            throw UnknownNameException(context->VariableName()->getSymbol());
        }

        throw ExpressionHasNotClassTypeException(context->getStart());
    }

    NameArray *visitNameNoGeneric(SanParser::NameNoGenericContext *context, std::shared_ptr<Scope> &scope)
    {
        auto name = context->VariableName()->getText();
        auto names = scope->get_names(name);

        if (!names->empty())
        {
            return names;
        }

        throw UnknownNameException(context->VariableName()->getSymbol());
    }

    NameArray *visitTypeNameClassGenerics(SanParser::ClassTypeNameGenericsContext *context, NameArray *names)
    {
        auto array = new NameArray();

        for (auto it = names->vector().rbegin(); it != names->vector().rend(); it++)
        {
            auto name = *it;

            if (auto alias = dynamic_cast<Alias *>(name))
            {
                auto values = this->visitTypeNameClassGenerics(context, alias->names);
                array->merge(values);
            }
            else if (auto generic_class = dynamic_cast<Types::GenericClassType *>(name))
            {
                auto generics = this->visitClassTypeNameGenerics(context);

                if (auto type = generic_class->get_child(generics))
                {
                    array->add(type);
                }
                else
                {
                    auto generated = this->generateGenericClassType(generic_class, generics);
                    array->add(generated);
                }
            }
            else if (auto generic_function = dynamic_cast<Types::GenericFunctionType *>(name))
            {
                auto generics = this->visitClassTypeNameGenerics(context);

                if (auto type = generic_function->get_child(generics))
                {
                    array->add(type);
                }
                else
                {
                    auto generated = this->generateGenericFunction(generic_function, generics);
                    array->add(generated);
                }
            }
            else if (auto generic_alias = dynamic_cast<Types::GenericAlias *>(name))
            {
                auto generics = this->visitClassTypeNameGenerics(context);

                if (auto alias = generic_alias->get_child(generics))
                {
                    array->add(alias);
                }
                else
                {
                    auto generated = this->generateGenericAlias(generic_alias, generics);
                    array->add(generated);
                }
            }

            if (array->empty())
            {
                throw NotAGenericException(context->getStart());
            }
        }

        return array;
    }

    Values::Constant *visitLiteralDeclaration(SanParser::LiteralDeclarationContext *context)
    {
        return this->visitLiteral(context->literal());
    }

    Values::Constant *visitLiteral(SanParser::LiteralContext *context)
    {
        auto scope = this->scopes.top();

        if (auto literal = context->booleanLiteral())
        {
            return this->visitBooleanLiteral(literal);
        }
        else if (const auto literal = context->integerLiteral())
        {
            return this->visitIntegerLiteral(literal);
        }
        else if (const auto literal = context->floatingLiteral())
        {
            return this->visitFloatingLiteral(literal);
        }
        else if (const auto literal = context->stringLiteral())
        {
            return this->visitStringLiteral(literal);
        }
        else if (const auto literal = context->CharLiteral())
        {
            auto str = literal->getSymbol()->getText();
            str = str.substr(1, str.size() - 2);

            int integer = 0;
            for (const auto &c : str)
            {
                integer = (integer << 8) + static_cast<int>(c);
            }

            auto type = scope->get_primary_type("i32");
            auto value = llvm::ConstantInt::get(type->get_ref(), integer);

            return new Values::Constant("literal_i32", type, value);
        }

        return nullptr;
    }

    std::string stringLiteralToString(const std::string &literal)
    {
        auto str = literal.substr(1, literal.size() - 2);

        str = std::regex_replace(str, std::regex("\\\\a"), "\a");
        str = std::regex_replace(str, std::regex("\\\\b"), "\b");
        str = std::regex_replace(str, std::regex("\\\\f"), "\f");
        str = std::regex_replace(str, std::regex("\\\\n"), "\n");
        str = std::regex_replace(str, std::regex("\\\\r"), "\r");
        str = std::regex_replace(str, std::regex("\\\\t"), "\t");
        str = std::regex_replace(str, std::regex("\\\\t"), "\t");
        str = std::regex_replace(str, std::regex("\\\\v"), "\v");
        str = std::regex_replace(str, std::regex("\\\\\\?"), "\?");
        str = std::regex_replace(str, std::regex("\\\\(.)"), "$1");

        return str;
    }

    std::string stringLiteralToString(SanParser::StringLiteralContext *context)
    {
        std::string str = "";

        for (auto *literal : context->StringLiteral())
        {
            str += this->stringLiteralToString(literal->getSymbol()->getText());
        }

        return str;
    }

    Values::Constant *visitBooleanLiteral(SanParser::BooleanLiteralContext *context)
    {
        auto scope = this->scopes.top();

        auto is_true = (context->True() != nullptr);

        auto type = Type::i1(scope->context());
        auto value = llvm::ConstantInt::get(type->get_ref(), is_true, false);

        return new Values::Constant(is_true ? "true" : "false", type, value);
    }

    Values::Constant *visitIntegerLiteral(SanParser::IntegerLiteralContext *context)
    {
        auto scope = this->scopes.top();

        const auto remove_digit_separators = [](std::string &str) {
            str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
            str.erase(std::remove(str.begin(), str.end(), '\''), str.end());
        };

        if (auto literal = context->DecimalLiteral())
        {
            auto str = literal->toString();
            remove_digit_separators(str);

            auto integer = std::stol(str);

            if (context->Sub())
            {
                integer *= -1;
            }

            auto type = Type::i32(scope->context());
            auto value = llvm::ConstantInt::get(type->get_ref(), integer, true);

            return new Values::Constant("literal_i32", type, value);
        }
        else if (context->ZeroLiteral())
        {
            auto type = Type::i32(scope->context());
            auto value = llvm::ConstantInt::get(type->get_ref(), 0, true);

            return new Values::Constant("literal_i32", type, value);
        }
        else if (auto literal = context->HexadecimalLiteral())
        {
            auto str = literal->toString();
            remove_digit_separators(str);

            auto integer = std::stol(str, nullptr, 16);

            if (context->Sub())
            {
                integer *= -1;
            }

            auto type = Type::i32(scope->context());
            auto value = llvm::ConstantInt::get(type->get_ref(), integer, false);

            return new Values::Constant("literal_i32", type, value);
        }
        else if (auto literal = context->BinaryLiteral())
        {
            auto str = literal->toString().substr(2);
            remove_digit_separators(str);

            auto integer = std::stol(str, nullptr, 2);

            if (context->Sub())
            {
                integer *= -1;
            }

            auto type = Type::i32(scope->context());
            auto value = llvm::ConstantInt::get(type->get_ref(), integer, false);

            return new Values::Constant("literal_i32", type, value);
        }

        return nullptr;
    }

    Values::Constant *visitFloatingLiteral(SanParser::FloatingLiteralContext *context)
    {
        auto scope = this->scopes.top();

        const auto remove_digit_separators = [](std::string &str) {
            str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
            str.erase(std::remove(str.begin(), str.end(), '\''), str.end());
        };

        auto str = context->FloatingLiteral()->toString();
        remove_digit_separators(str);

        auto floating = std::stod(str);

        if (context->Sub())
        {
            floating *= -1;
        }

        auto type = Type::f64(scope->context());
        auto value = llvm::ConstantFP::get(type->get_ref(), floating);

        return new Values::Constant("literal_f64", type, value);
    }

    Values::GlobalConstant *visitStringLiteral(SanParser::StringLiteralContext *context)
    {
        auto scope = this->scopes.top();

        std::string str = this->stringLiteralToString(context);

        auto constant = llvm::ConstantDataArray::getString(this->env.llvm_context, str, true);

        auto type = scope->get_primary_type("i8")->array(str.size() + 1);
        auto value = Values::GlobalConstant::create(".str", type, constant, scope->module());

        return value;
    }

    Type *visitType(SanParser::TypeContext *context)
    {
        auto scope = this->scopes.top();
        auto type = this->visitTypeName(context->typeName());

        for (const auto &pointer : context->typePointer())
        {
            type = type->pointer();
        }

        if (context->typeReference())
        {
            type = type->reference();
        }

        return type;
    }

    Type *visitTypeName(SanParser::TypeNameContext *context)
    {
        if (auto scoped_name_context = context->scopedName())
        {
            auto scoped_name = this->visitScopedName(scoped_name_context);
            return this->typeFromName(scoped_name, context);
        }
        else if (auto function_type_context = context->functionType())
        {
            return this->visitFunctionType(function_type_context);
        }

        return nullptr;
    }

    Type *visitFunctionType(SanParser::FunctionTypeContext *context)
    {
        auto scope = this->scopes.top();

        auto args = this->visitFunctionArguments(context->functionArguments());
        auto is_variadic = (context->functionVariadicArgument() != nullptr);

        Type *return_type = nullptr;

        if (auto type_context = context->type())
        {
            return_type = this->visitType(type_context);
        }
        else
        {
            return_type = Type::voidt(scope->context());
        }

        return Types::FunctionType::create(scope->builder(), scope->module(), "", return_type, args, is_variadic, false)->pointer();
    }

    Types::ClassType *visitClassTypeName(SanParser::ClassTypeNameContext *context)
    {
        auto name = this->visitScopedName(context->scopedName());
        auto type = this->typeFromName(name, context);

        if (auto class_type = dynamic_cast<Types::ClassType *>(type))
        {
            return class_type;
        }

        throw NotAClassException(context->getStart());
    }

    std::vector<Type *> visitClassTypeNameGenerics(SanParser::ClassTypeNameGenericsContext *context)
    {
        auto scope = this->scopes.top();

        std::vector<Type *> types;

        for (auto &type_context : context->type())
        {
            auto type = this->visitType(type_context);
            types.push_back(type);
        }

        return types;
    }

    Attributes visitAttributes(SanParser::AttributesContext *context)
    {
        Attributes attributes(this->env);

        for (auto &attribute_context : context->attribute())
        {
            auto attribute = this->visitAttribute(attribute_context);
            attributes.set(attribute);
        }

        return attributes;
    }

    std::pair<std::string, std::string> visitAttribute(SanParser::AttributeContext *context)
    {
        auto key = context->VariableName()->getText();

        if (auto literal = context->StringLiteral())
        {
            auto value = this->stringLiteralToString(literal->getText());
            return std::make_pair(key, value);
        }

        return std::make_pair(key, "true");
    }

    void visitAssemblyStatement(SanParser::AssemblyStatementContext *context)
    {
        auto scope = this->scopes.top();

        auto code = this->stringLiteralToString(context->stringLiteral());

        std::vector<AssemblyOperand> outputs = this->visitAssemblyOutputs(context->assemblyOutput());
        std::vector<AssemblyOperand> inputs = this->visitAssemblyInputs(context->assemblyInput());
        std::string clobbers = this->visitAssemblyClobbers(context->assemblyClobber());

        if (!clobbers.empty())
        {
            clobbers += ",";
        }

        std::string operands_clobbers = "";

        std::vector<Value *> output_values;
        std::vector<llvm::Type *> output_types;

        std::vector<Value *> output_args;
        std::vector<Value *> input_args;

        for (auto &output : outputs)
        {
            if (output.type == AssemblyConstraintModifier::ReadWrite)
            {
                auto loaded = output.expression->load(scope->builder());
                output_args.push_back(loaded);

                output.name[0] = '=';
            }

            operands_clobbers += output.name + ",";

            output_types.push_back(output.expression->type->get_ref());
            output_values.push_back(output.expression);
        }

        for (auto &input : inputs)
        {
            auto loaded = input.expression->load(scope->builder());
            input_args.push_back(loaded);
            operands_clobbers += input.name + ",";
        }

        for (size_t i = 0; i < outputs.size(); i++)
        {
            if (outputs[i].type == AssemblyConstraintModifier::ReadWrite)
            {
                operands_clobbers += std::to_string(i) + ",";
            }
        }

        std::vector<Value *> args;
        std::vector<Types::FunctionArgument> function_args;

        for (auto &arg : input_args)
        {
            args.push_back(arg);
            function_args.push_back(Types::FunctionArgument("", arg->type));
        }

        for (auto &arg : output_args)
        {
            args.push_back(arg);
            function_args.push_back(Types::FunctionArgument("", arg->type));
        }

        llvm::Type *return_type = Type::llvm_void(scope->context());

        if (output_types.size() == 1)
        {
            return_type = output_types[0];
        }
        else if (!output_types.empty())
        {
            return_type = llvm::StructType::get(scope->context(), output_types);
        }

        auto type = Types::FunctionType::create(scope->builder(), scope->module(), "inline.asm", new Type(".tmp.class", return_type), function_args, false, false, false);

        auto value = llvm::InlineAsm::get(type->get_ref(), code, operands_clobbers + clobbers + "~{dirflag},~{fpsr},~{flags}", true);
        auto ret = Value("inline.asm", type, value).call(scope->builder(), scope->module(), args);

        if (return_type->isStructTy())
        {
            auto builder = scope->builder();
            auto ref = ret->get_ref();

            for (size_t i = 0; i < output_values.size(); i++)
            {
                auto value = builder.CreateExtractValue(ref, i, "");
                builder.CreateStore(value, output_values[i]->get_ref());
            }
        }
        else if (!return_type->isVoidTy())
        {
            scope->builder().CreateStore(ret->get_ref(), output_values[0]->get_ref());
        }
    }

    std::vector<AssemblyOperand> visitAssemblyOutputs(const std::vector<SanParser::AssemblyOutputContext *> &context)
    {
        std::vector<AssemblyOperand> operands;

        for (auto &clobber : context)
        {
            auto output = this->visitAssemblyOutput(clobber);
            operands.push_back(output);
        }

        return operands;
    }

    AssemblyOperand visitAssemblyOutput(SanParser::AssemblyOutputContext *context)
    {
        auto name = this->stringLiteralToString(context->StringLiteral()->getText());
        auto value = this->valueFromExpression(context->expression());

        if (!value->is_alloca)
        {
            throw InvalidLeftValueException(context->expression()->getStart());
        }

        auto operand = this->createAssemblyOperand(name, value);

        if (operand.type == AssemblyConstraintModifier::None)
        {
            throw InvalidInputConstraintException(context->StringLiteral()->getSymbol());
        }

        return operand;
    }

    std::vector<AssemblyOperand> visitAssemblyInputs(const std::vector<SanParser::AssemblyInputContext *> &context)
    {
        std::vector<AssemblyOperand> operands;

        for (auto &clobber : context)
        {
            auto input = this->visitAssemblyInput(clobber);
            operands.push_back(input);
        }

        return operands;
    }

    AssemblyOperand visitAssemblyInput(SanParser::AssemblyInputContext *context)
    {
        auto name = this->stringLiteralToString(context->StringLiteral()->getText());
        auto value = this->valueFromExpression(context->expression());

        auto operand = this->createAssemblyOperand(name, value);

        return operand;
    }

    AssemblyOperand createAssemblyOperand(const std::string &name, Value *lvalue)
    {
        AssemblyOperand operand(AssemblyConstraintModifier::None, name, lvalue);

        if (name[0] == '=')
        {
            operand.type = AssemblyConstraintModifier::WriteOnly;
        }
        else if (name[0] == '+')
        {
            operand.type = AssemblyConstraintModifier::ReadWrite;
        }

        if (name[1] == '&')
        {
            operand.is_earlyclobber = true;
        }

        return operand;
    }

    std::string visitAssemblyClobbers(const std::vector<SanParser::AssemblyClobberContext *> &context)
    {
        std::string clobbers = "";

        for (auto &clobber : context)
        {
            if (!clobbers.empty())
            {
                clobbers += ",";
            }

            clobbers += this->visitAssemblyClobber(clobber);
        }

        return clobbers;
    }

    std::string visitAssemblyClobber(SanParser::AssemblyClobberContext *context)
    {
        return "~{" + this->stringLiteralToString(context->StringLiteral()->getText()) + "}";
    }

    Name *visitAlias(SanParser::AliasContext *context)
    {
        auto scope = this->scopes.top();

        auto name = context->VariableName()->getText();

        if (auto generics_context = context->classGenerics())
        {
            auto generics = this->visitClassGenerics(generics_context);

            auto alias_scope = Scope::create(scope);
            auto alias = new Types::GenericAlias(alias_scope, name, generics, context);

            scope->add_name(name, alias);
            return alias;
        }

        auto alias = this->generateAlias(context);

        scope->add_name(name, alias);
        return alias;
    }

    Alias *generateGenericAlias(Types::GenericAlias *generic, const std::vector<Type *> &generics)
    {
        auto scope = Scope::create(generic->scope);
        this->scopes.push(scope);

        for (size_t i = 0; i < generic->generics.size(); i++)
        {
            auto &name = generic->generics[i]->name;
            scope->add_name(name, generics[i]);
        }

        auto alias = this->generateAlias(generic->context);

        this->scopes.pop();

        generic->children.push_back(Types::GenericAliasChild(generics, alias));

        return alias;
    }

    Alias *generateAlias(SanParser::AliasContext *context)
    {
        auto name = context->VariableName()->getText();
        NameArray *names = nullptr;

        if (auto scoped_name_context = context->scopedName())
        {
            names = this->visitScopedName(scoped_name_context);
        }
        else if (auto type_context = context->type())
        {
            auto type = this->visitType(type_context);
            names = new NameArray({type});
        }
        else if (auto literal_context = context->literal())
        {
            auto literal = this->visitLiteral(literal_context);
            names = new NameArray({literal});
        }

        return new Alias(name, names);
    }
};
} // namespace San
