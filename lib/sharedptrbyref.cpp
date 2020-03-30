#include "sharedptrbyref.h"

#include "astutils.h"
#include "errorlogger.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"

#include <list>

// Register this check class (by creating a static instance of it)
namespace {
    CheckSharedPtrPassedByRef instance;
}

void CheckSharedPtrPassedByRef::sharedPtrByRef()
{
    // Read all symbols from cppcheck pre-processing.
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    // Iterate over functions.
    for (const auto scope : symbolDatabase->functionScopes)
    {
        // Retrieve function instance.
        const Function* function = scope->function;
        if (!function)
        {
            // Invalid function instance. Continue.
            continue;
        }

        // Iterate over all the arguments on the functions' argument list.
        for (const auto& argument : function->argumentList)
        {
            // Check whether argument's type name matches expected types names of shared_ptr.
            auto it = std::find_if(m_sharedPtrTypesNames.begin(), m_sharedPtrTypesNames.end(),
                [&argument](const auto& name) {
                    return argument.getTypeName() == name;
            });
            if (it != m_sharedPtrTypesNames.end())
            {
                // Argument of type std::shared_ptr or shared_ptr found.

                if (argument.isReference())
                {
                    // Additionally, it's a reference type.
                    // Report the error that there's a reference to shared_ptr on the arguments list of a function.
                    errorSharedPtrPassedByReference(argument.nameToken(), argument.name());
                }
            }
        }
    }
}

void CheckSharedPtrPassedByRef::errorSharedPtrPassedByReference(const Token* tok, const std::string& paramName)
{
    reportError(tok, Severity::warning, "SharedPtrPassedByRef", "Shared_ptr shall not be passed to a function by reference.");
}