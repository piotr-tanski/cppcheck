#ifndef invocableasfunctionargH
#define invocableasfunctionargH

#include "check.h"
#include "config.h"

class ErrorLogger;
class Settings;
class Token;
class Tokenizer;

class CPPCHECKLIB CheckInvocableAsFunctionArgument : public Check
{
public:
    CheckInvocableAsFunctionArgument();
    CheckInvocableAsFunctionArgument(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger);

    void runChecks(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger) OVERRIDE;
    void invocableAsFunctionArg();

private:
    void findInvocablesOnArgList(const Token* callToken, const Token* firstArg);

    void getErrorMessages(ErrorLogger* errorLogger, const Settings* settings) const OVERRIDE;
    void errorInvocableAsFunctionArg(const Token* tok, const std::string& argName);

    static std::string name()
    {
        return "InvocableAsFunctionArgument";
    }

    std::string classInfo() const OVERRIDE
    {
        return "Check for function calls placed in the arguments list of another function call. Only literals as arguments shall be allowed.";
    }
};

#endif //invocableasfunctionargH
