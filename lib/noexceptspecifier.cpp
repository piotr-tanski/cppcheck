#include "noexceptspecifier.h"

#include "errorlogger.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"

namespace
{
    CheckExceptionSpecifier instance;
} // namespace

CheckExceptionSpecifier::CheckExceptionSpecifier() : Check{name()}
{}

CheckExceptionSpecifier::CheckExceptionSpecifier(const Tokenizer *tokenizer,
                                                 const Settings *settings,
                                                 ErrorLogger *errorLogger)
                                                 : Check{name(), tokenizer, settings, errorLogger}
{}

void CheckExceptionSpecifier::runChecks(const Tokenizer *tokenizer,
                                        const Settings *settings,
                                        ErrorLogger *errorLogger)
{
    CheckExceptionSpecifier check{tokenizer, settings, errorLogger};
    check.noexceptSpecialMemberFunctions();
    check.noexceptGetters();
    check.noexceptFunctionsReturningLiterals();
}

void CheckExceptionSpecifier::noexceptSpecialMemberFunctions()
{
    const auto symbolDb = mTokenizer->getSymbolDatabase();

    // Iterate over all classes and structs.
    for (const auto scope : symbolDb->classAndStructScopes)
    {
        for (const auto& func : scope->functionList)
        {
            if (isDestructor(func)
                || isMoveConstructor(func)
                || isMoveAssignmentOperator(func, scope))
            {
                if (!func.isNoExcept())
                {
                    noexceptErrorMessage(func.token);
                }
            }
        }
    }
}

void CheckExceptionSpecifier::noexceptGetters()
{
    const auto symbolDb = mTokenizer->getSymbolDatabase();

    // Iterate over all classes and structs.
    for (const auto scope : symbolDb->classAndStructScopes)
    {
        for (auto& func : scope->functionList)
        {
            if (isGetter(func, scope)) {
                if (!func.isNoExcept()) {
                    noexceptErrorMessage(func.token);
                }
                if (!func.isConst()) {
                    constnessErrorMessage(func.token);
                }
            }
        }
    }
}

void CheckExceptionSpecifier::noexceptFunctionsReturningLiterals()
{
    const auto symbolDb = mTokenizer->getSymbolDatabase();

    // Iterate over all functions.
    for (const auto scope : symbolDb->functionScopes) {
        const auto function = scope->function;
        if (function != nullptr && isFunctionReturningLiteral(function)) {
            if (!function->isNoExcept()) {
                noexceptErrorMessage(function->token);
            }
        }
    }
}

bool CheckExceptionSpecifier::isDestructor(const Function &function) const noexcept
{
    return function.type == Function::eDestructor;
}

bool CheckExceptionSpecifier::isMoveConstructor(const Function& function) const noexcept
{
    return function.type == Function::eMoveConstructor;
}

bool CheckExceptionSpecifier::isMoveAssignmentOperator(const Function& function,
                                                       const Scope* classScope) const noexcept
{
    if (function.type == Function::eOperatorEqual) {
        const Variable* argument = function.getArgumentVar(0);
        return argument != nullptr
               && argument->type() != nullptr
               && argument->isRValueReference()
               && argument->type()->classScope == classScope;
    }
    return false;
}

bool CheckExceptionSpecifier::isGetter(const Function &function, const Scope *classScope) const noexcept
{
    return function.type == Function::eFunction
           && isNoexceptReturnType(function)
           && function.hasBody()
           && (returnsMember(function, classScope) || returnsLiteral(function));
}

bool CheckExceptionSpecifier::isNoexceptReturnType(const Function &function) const noexcept
{
    return returnsConstReference(function)
            || returnsConstPointer(function)
            || returnsIntegralType(function);
}

bool CheckExceptionSpecifier::returnsConstReference(const Function &function) const noexcept
{
    const auto defStart = function.retDef;
    const auto defEnd = function.returnDefEnd();
    if (Token::Match(defStart, "const") && defEnd->strAt(-1) == "&") {
        return true;
    }
    return false;
}

bool CheckExceptionSpecifier::returnsConstPointer(const Function &function) const noexcept
{
    const auto defStart = function.retDef;
    const auto defEnd = function.returnDefEnd();
    if (Token::Match(defStart, "const") && defEnd->strAt(-1) == "*") {
        return true;
    }
    return false;
}

bool CheckExceptionSpecifier::returnsIntegralType(const Function &function) const noexcept
{
    static constexpr char integralTypes[] = "bool|_Bool|char|wchar_t|int|short|long|float|double";
    static constexpr char fixedWidthIntegralTypes[] = "int8_t|int16_t|int32_t|int64_t|uint8_t|uint16_t|uint32_t|uint64_t";

    const auto type = function.retType;
    if (type != nullptr && type->isEnumType()) {
        return true;
    }
    else if (function.retDef != nullptr) {
        if (Token::Match(function.retDef, "signed|unsigned")) {
            return true;
        }
        if (Token::Match(function.retDef, "std ::")) {
            return Token::Match(function.retDef->tokAt(2), fixedWidthIntegralTypes);
        }
        return Token::Match(function.retDef, integralTypes);
    }
    return false;
}

bool CheckExceptionSpecifier::returnsMember(const Function &function,
                                                     const Scope *classScope) const noexcept
{
    const auto funcScope = function.functionScope;
    for (auto tok = funcScope->bodyStart; tok != funcScope->bodyEnd; tok = tok->next()) {
        if (Token::Match(tok, "{|}")) {
            // Skip braces
            continue;
        }

        // Search for variables.
        if (Token::Match(tok, "return &|*| %var% ;")) {
            if (tok->strAt(1) == "&" || tok->strAt(1) == "*") {
                tok = tok->tokAt(1);
            }
            const auto variable = tok->tokAt(1)->variable();
            if (variable != nullptr && variable->scope() == classScope) {
                return true;
            }
        }

        // Only return statement is expected inside the function - break and return false.
        break;
    }
    return false;
}

bool CheckExceptionSpecifier::returnsLiteral(const Function &function) const noexcept
{
    const auto funcScope = function.functionScope;
    for (auto tok = funcScope->bodyStart; tok != funcScope->bodyEnd; tok = tok->next()) {
        if (Token::Match(tok, "{|}")) {
            // Skip braces
            continue;
        }

        // Search for numbers, characters and booleans literals.
        if (Token::Match(tok, "return %num%|%bool%|%char% ;")) {
            return true;
        }

        // Search for enums, e.g. Class::Enum::E1.
        if (Token::Match(tok, "return")) {
            auto returnToken = tok->tokAt(1);
            while (Token::Match(returnToken, "%type% ::")) {
                returnToken = returnToken->tokAt(2);
            }
            if (returnToken->tokAt(1)->str() == ";") {
                const auto value = returnToken->valueType();
                if (value != nullptr && value->isEnum()) {
                    return true;
                }
            }
        }

        // Only return statement is expected inside the function - break and return false.
        break;
    }
    return false;
}

bool CheckExceptionSpecifier::isFunctionReturningLiteral(const Function *function) const noexcept
{
    return returnsIntegralType(*function)
            && function->hasBody()
            && returnsLiteral(*function);
}

void CheckExceptionSpecifier::getErrorMessages(ErrorLogger *errorLogger,
                                               const Settings *settings) const
{
    CheckExceptionSpecifier check{nullptr, settings, errorLogger};
    check.noexceptErrorMessage(nullptr);
    check.constnessErrorMessage(nullptr);
}

void CheckExceptionSpecifier::noexceptErrorMessage(const Token *token)
{
    reportError(token, Severity::warning, name(), "The function shall be specified noexcept.");
}

void CheckExceptionSpecifier::constnessErrorMessage(const Token *token)
{
    reportError(token, Severity::warning, name(), "The function shall be specified const.");
}