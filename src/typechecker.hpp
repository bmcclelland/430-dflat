#ifndef TYPECHECKER_HPP
#define TYPECHECKER_HPP

#include "asn.hpp"
#include "typechecker_tools.hpp"

namespace dflat
{

#define cast(ptr,type) dynamic_cast<type*>(ptr.get())

//Runner Functions:
TypeEnv typeCheck(Vector<ASNPtr> const&);
Type typeCheck(ASNPtr const&);

//Helper Functions:
Type lookupRuleType(TypeEnv const&, String const&);
Vector<Type> lookupMethodType(TypeEnv const&, String const&);
Vector<Type> lookupMethodTypeByClass(TypeEnv const&, String const&, String const&);
Type lookupVarType(TypeEnv const&, String const&);
Type lookupVarTypeByClass(TypeEnv const&, String const&, String const&);
bool validType(TypeEnv const&, String const&);
void assertTypeIs(Type const&, Type const&);
String funcCanonicalName(String const&, Vector<Type> const&);
String unopCanonicalName(OpType, Type const&);
String binopCanonicalName(OpType, Type const&, Type const&);
TypeEnv initialTypeEnv();

//TypeChecker Exceptions class:
class TypeCheckerException : public std::exception
{
    public:
        TypeCheckerException(String msg) noexcept;
        const char* what() const noexcept;
    private:
        String message;
};

} //namespace dflat

#endif // TYPECHECKER_HPP
