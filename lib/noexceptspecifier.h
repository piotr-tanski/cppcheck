#ifndef noexceptmoveH
#define noexceptmoveH

#include "check.h"
#include "config.h"

#include <vector>

class ErrorLogger;
class Function;
class Scope;
class Settings;
class Token;
class Tokenizer;

class CPPCHECKLIB CheckExceptionSpecifier : public Check
{
public:
    CheckExceptionSpecifier();
    CheckExceptionSpecifier(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger);

    void runChecks(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger) OVERRIDE;
    void noexceptSpecialMemberFunctions();
    void noexceptGetters();
    void noexceptFunctionsReturningLiterals();
    void noexceptFunctionsNotCallingThrowingFunctions();

private:
    bool isDestructor(const Function& function) const noexcept;
    bool isMoveConstructor(const Function& function) const noexcept;
    bool isMoveAssignmentOperator(const Function& function, const Scope* classScope) const noexcept;

    bool isGetter(const Function& function, const Scope *classScope) const noexcept;
    bool isNoexceptReturnType(const Function& function) const noexcept;
    bool returnsConstReference(const Function& function) const noexcept;
    bool returnsConstPointer(const Function& function) const noexcept;
    bool returnsConstCharPointer(const Function& function) const noexcept;
    bool returnsIntegralType(const Function& function) const noexcept;
    bool returnsMember(const Function& function, const Scope *classScope) const noexcept;
    bool returnsLiteral(const Function& function) const noexcept;
    bool returnsVoid(const Function& function) const noexcept;
    bool returnsPointer(const Function& function) const noexcept;
    bool returnsReference(const Function& function) const noexcept;

    bool isFunctionReturningLiteral(const Function* function) const noexcept;

    bool doesntThrow(const Function& function) const noexcept;
    bool doesntThrowFromNestedFunctions(const Function& function) const noexcept;
    std::vector<Scope*> findCatchAllScopes(const Function& function) const noexcept;
    bool isNestedInCatchAll(const Token *token, const std::vector<Scope*> catchAllScopes) const noexcept;
    bool isSpecialMemberFunction(const Function& function) const noexcept;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const OVERRIDE;
    void noexceptErrorMessage(const Token* token);
    void constnessErrorMessage(const Token* token);

    static std::string name()
    {
        return "CheckExceptionSpecifier";
    }

    std::string classInfo() const OVERRIDE
    {
        return "Check for functions that could be noexcept.";
    }
};

#endif //noexceptmoveH
