#include "invocableasfunctionarg.h"

#include "errorlogger.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"

namespace
{
    CheckInvocableAsFunctionArgument instance;
} // namespace

CheckInvocableAsFunctionArgument::CheckInvocableAsFunctionArgument() : Check(name())
{}

CheckInvocableAsFunctionArgument::CheckInvocableAsFunctionArgument(const Tokenizer *tokenizer, const Settings *settings,
                                                                   ErrorLogger *errorLogger)
                                                                   : Check(name(), tokenizer, settings, errorLogger)
{}

void CheckInvocableAsFunctionArgument::runChecks(const Tokenizer *tokenizer, const Settings *settings,
                                                 ErrorLogger *errorLogger)
{
    CheckInvocableAsFunctionArgument check{tokenizer, settings, errorLogger};
    check.invocableAsFunctionArg();
}

void CheckInvocableAsFunctionArgument::invocableAsFunctionArg()
{
    const auto symbolDb = mTokenizer->getSymbolDatabase();

    // Iterate over all functions.
    for (const auto scope : symbolDb->functionScopes)
    {
        // Iterate over function's body.
        for (auto it = scope->bodyStart; it != scope->bodyEnd; it = it->next())
        {
            // Check for a pattern of a function/method/constructor call.
            if (!Token::Match(it, "%name% (|{"))
                continue;

            const auto func = it->function();
            if (func != nullptr && func->isVariadic()) {
                findInvocablesOnArgList(it, it->tokAt((func->argCount() - 1) + 2));
            }
        }
    }

    // Iterate over class/struct/union/global/namespace scopes.
    for (const auto& scope : symbolDb->scopeList)
    {
        if (scope.isClassOrStructOrUnion()
            || scope.type == Scope::eGlobal
            || scope.type == Scope::eNamespace)
        {
            for (auto tok = scope.bodyStart; tok != scope.bodyEnd; tok = tok->next())
            {
                if (!Token::Match(tok, "%name% (|{"))
                    continue;

                const auto func = tok->function();
                if (func != nullptr && func->isVariadic()) {
                    findInvocablesOnArgList(tok, tok->tokAt((func->argCount() - 1) + 2));
                }
            }
        }
    }
}

void CheckInvocableAsFunctionArgument::findInvocablesOnArgList(const Token* callToken, const Token* firstArg)
{
    for (auto arg = firstArg; arg != nullptr; arg = arg->nextArgument())
    {
        const auto isArgAFunctionCall = (arg->function() != nullptr);
        if (isArgAFunctionCall)
        {
            errorInvocableAsFunctionArg(arg, callToken->str());
        }
    }
}

void CheckInvocableAsFunctionArgument::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckInvocableAsFunctionArgument check{nullptr, settings, errorLogger};
    check.errorInvocableAsFunctionArg(nullptr, "arg");
}

void CheckInvocableAsFunctionArgument::errorInvocableAsFunctionArg(const Token *argTok, const std::string &functionName)
{
    reportError(argTok, Severity::warning, name(),
                "Invocable \"" + argTok->str() + "\" shall not be an argument to variadic part of \"" + functionName + "\" function. Only literals or variables are allowed.");
}