#include "asn.hpp"
#include "config.hpp"
#include <iostream>

namespace dflat
{

void VariableExp::generateCode(GenEnv & env)
{
    if (object)
    {
        // object->member
        env.emitObject(*object, name);
    }
    else
    {
        if (env.lookupLocalType(name))
        {
            env << CodeVarName(name);
        }
        else
        {
            // Must be using implicit this.
            // TODO this logic might be a source of errors.
            env.emitObject(config::thisName, name);
        }
    }
}

void NumberExp::generateCode(GenEnv & env)
{
    env << CodeNumber(value);
}

void BoolExp::generateCode(GenEnv & env)
{
    env << CodeNumber(value ? 1 : 0);
}

void BinopExp::generateCode(GenEnv & env)
{
    env << CodeLiteral("(")
        << lhs
        << CodeLiteral(opString(op))
        << rhs
        << CodeLiteral(")");
}

void UnopExp::generateCode(GenEnv & env)
{
    env << CodeLiteral("(") 
        << CodeLiteral(opString(op))
        << rhs
        << CodeLiteral(")");
}

void Block::generateCode(GenEnv & env)
{
    env.enterScope();

    env << CodeTabs()
        << CodeLiteral("{\n")
        << CodeTabIn();

    for (ASNPtr const& stmt : statements)
    {
        env << stmt;
    }

    env << CodeTabOut()
        << CodeTabs()
        << CodeLiteral("}\n\n");

    env.leaveScope();
}

void IfStm::generateCode(GenEnv & env)
{
    env << CodeTabs()
        << CodeLiteral("if (")
        << logicExp
        << CodeLiteral(")\n")
        << trueStatements;

    if (hasFalse)
    {
        env << CodeTabs()
            << CodeLiteral("else\n")
            << falseStatements;
    }
}

void WhileStm::generateCode(GenEnv & env)
{
    env << CodeTabs()
        << CodeLiteral("while (")
        << logicExp
        << CodeLiteral(")\n")
        << statements;
}

void MethodDef::generateCode(GenEnv & env)
{
    CanonName const canonName(name, asnType->method());
    env.enterMethod(canonName);
    ValueType curClass = env.curClass().type;
    
    env << CodeTabs()
        << CodeTypeName(retTypeName)
        << CodeLiteral(" ")
        << CodeMethodName(curClass, canonName)
        << CodeLiteral("(")
        << CodeTypeName(curClass.name())
        << CodeLiteral(" ")
        << CodeVarName(config::thisName);
        
    for(auto&& ar : args)
    {
        env << CodeLiteral(", ")
            << CodeTypeName(ar.typeName)
            << CodeLiteral(" ")
            << CodeVarName(ar.name);
        
        env.declareLocal(ar.name, ValueType(ar.typeName));
    }

    env << CodeLiteral(")\n")
        << statements;

    env.leaveMethod();
}

void MethodExp::generateCode(GenEnv & env)
{
    String const objectName = method.object ? *method.object 
                                            : config::thisName;

    auto [thisType, methodName] = env.getMethodMeta(this);
    ValueType objectType = env.getLocalType(objectName);

    env << CodeMethodName(thisType, methodName)
        << CodeLiteral("(");

    // Base-cast if necessary.
    if (objectType != thisType)
    {
        env << CodeLiteral("(")
            << CodeTypeName(thisType.toString())
            << CodeLiteral(")");
    }

    env << CodeVarName(objectName);
    
    for (auto&& ar : args)
    {
        env << CodeLiteral(", ")
            << ar;
        
        ar->generateCode(env);
    }
   
    env << CodeLiteral(")");
}

void MethodStm::generateCode(GenEnv& env)
{
    env << CodeTabs()
        << methodExp
        << CodeLiteral(";\n");
}

void NewExp::generateCode(GenEnv& env)
{
    // DF_NEW(T) is a macro equivalent to "(struct T*)malloc(sizeof(T))"
    env << CodeLiteral("DF_NEW(")
        << CodeClassDecl(typeName)
        << CodeLiteral(")");

    // TODO Constructors taking args.
}

void AssignStm::generateCode(GenEnv& env)
{
    // TODO: check if this works for every case (i.e. rhs is new stm)
    env << CodeTabs()
        << lhs
        << CodeLiteral(" = ")
        << rhs
        << CodeLiteral(";\n");
}

void VarDecStm::generateCode(GenEnv& env)
{
    env << CodeTabs()
        << CodeTypeName(typeName)
        << CodeLiteral(" ")
        << CodeVarName(name)
        << CodeLiteral(";\n");

    if (env.inMethod())
    {
        env.declareLocal(name, ValueType(typeName));
    }
}

void VarDecAssignStm::generateCode(GenEnv& env)
{
    env << CodeTabs()
        << CodeTypeName(typeName)
        << CodeLiteral(" ")
        << CodeVarName(name)
        << CodeLiteral(" = ")
        << value
        << CodeLiteral(";\n");

    if (env.inMethod())
    {
        env.declareLocal(name, ValueType(typeName)); 
    }
}

void RetStm::generateCode(GenEnv& env)
{
    env << CodeTabs()
        << CodeLiteral("return ")
        << value
        << CodeLiteral(";\n");
}

void ClassDecl::generateCode(GenEnv& env)
{
    env.enterClass(ValueType(name));
                       
    env << CodeLiteral("struct ")
        << CodeClassDecl(name)
        << CodeLiteral("\n{\n");

    if(parent) 
    {
        env << CodeTabs()
            << CodeLiteral("struct ")
            << CodeClassDecl(parent->name)
            << CodeLiteral(" ")
            << CodeParent()
            << CodeLiteral(";\n");
    }

    for (ASNPtr& member : members)
    {
        env << member;
    }

    env << CodeLiteral("};\n\n");
    env.leaveClass();
}

} //namespace dflat
