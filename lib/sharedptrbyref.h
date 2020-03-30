#ifndef sharedptrbyrefH
#define sharedptrbyrefH

#include "check.h"
#include "config.h"

#include <string>
#include <vector>

// Forward declarations.
class ErrorLogger;
class Settings;
class Token;
class Tokenizer;

// CPPCHECKLIB provides the information whether a class symbol shall be exported.
class CPPCHECKLIB CheckSharedPtrPassedByRef : public Check
{
public:
    // Default constructor.
    CheckSharedPtrPassedByRef() : Check(name())
    {}

    // Constructor needed to properly initialize the Check.
    CheckSharedPtrPassedByRef(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger)
        : Check{name(), tokenizer, settings, errorLogger}
    {
    }

    // Function performing all the steps of the check.
    void runChecks(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger) OVERRIDE
    {
        CheckSharedPtrPassedByRef check{tokenizer, settings, errorLogger};
        check.sharedPtrByRef();
    }

    // Function defining a step.
    // It performs the search for all shared_ptr passed by ref.
    void sharedPtrByRef();

private:
    // Internal function reporting the error.
    void errorSharedPtrPassedByReference(const Token* tok, const std::string& paramName);

    // Register all possible error messages of the check.
    void getErrorMessages(ErrorLogger* errorLogger, const Settings* settings) const OVERRIDE {
        CheckSharedPtrPassedByRef c(nullptr, settings, errorLogger);
        c.errorSharedPtrPassedByReference(nullptr, "arg1");
    }

    // Function returning the name of the rule.
    static std::string name()
    {
        return "SharedPtrPassedByRef";
    }

    // Function returning the description of the rule.
    std::string classInfo() const OVERRIDE
    {
        return "Check for shared_ptrs passed to a function by reference instead of copy";
    }

    // Helper structure containing possible names of shared_ptr type.
    std::vector<std::string> m_sharedPtrTypesNames{"std::shared_ptr", "shared_ptr"};
};

#endif // sharedptrbyrefH
