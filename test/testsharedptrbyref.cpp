#include "sharedptrbyref.h"

#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

#include <iostream>

class TestSharedPtrByRef : public TestFixture {
public:
    TestSharedPtrByRef() : TestFixture("TestSharedPtrByRef") {}

private:
    Settings settings;

    void check(const char code[]) {
        errout.str("");

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        CheckSharedPtrPassedByRef check;
        check.runChecks(&tokenizer, &settings, this);
    }

    void run() OVERRIDE {
        settings.addEnabled("warning");

        TEST_CASE(StdSharedPtrPassedByRefToFunction);
        TEST_CASE(SharedPtrPassedByRefToFunction);
        TEST_CASE(StdSharedPtrPassedByCopyToFunction);
    }

    void StdSharedPtrPassedByRefToFunction() {
        check("void fn(int x, std::shared_ptr<A>& ptr, int y) {\n}");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Shared_ptr shall not be passed to a function by reference.\n", errout.str());
    }

    void SharedPtrPassedByRefToFunction() {
        check("void fn(int x, shared_ptr<A>& ptr, int y) {\n}");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Shared_ptr shall not be passed to a function by reference.\n", errout.str());
    }

    void StdSharedPtrPassedByCopyToFunction() {
        check("void fn(int x, std::shared_ptr<A> ptr, int y) {\n}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestSharedPtrByRef)
