#include "invocableasfunctionarg.h"

#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

class TestInvocableAsFunctionArg : public TestFixture
{
public:
    TestInvocableAsFunctionArg() : TestFixture("TestInvocableAsFunctionArg")
    {}

private:
    Settings settings;

    void run() OVERRIDE
    {
        settings.addEnabled("warning");

        TEST_CASE(functionCall);
        TEST_CASE(constructorCall);
        TEST_CASE(methodCall);
    }

    void check(const std::string& code)
    {
        errout.str({});

        Tokenizer tokenizer{&settings, this};
        std::istringstream istr{code};
        tokenizer.tokenize(istr, "test.cpp");

        CheckInvocableAsFunctionArgument check;
        check.runChecks(&tokenizer, &settings, this);
    }

    void functionCall()
    {
        check(R"(
                void func(int x, int y)
                {}
                int main() {
                    func(0, 1);
                    return 0;
                })");
        ASSERT_EQUALS("", errout.str());

        check(R"(
                int f1() {
                    return 0;
                }
                void func(int x, int y) {}

                int main() {
                    func(f1(), 1);
                    return 0;
                })");
        ASSERT_EQUALS("[test.cpp:8]: (warning) Invocable \"f1\" shall not be an argument to \"func\" function/method/constructor. Only literals or variables are allowed.\n",
                      errout.str());

        check(R"(
            class X { };
            X f1() {
                return X{};
            }
            int f2() {
                return 0;
            }

            void func(const X& x, int a, int b, int c) {}

            int main() {
                func(X{}, 0, 1, 2);
                func(f1(), 0, 1, 2);
                func(X{}, 0, f2(), 2);
                func(f1(), f2(), 1, 2);

                const X& x = f1();
                int a = f2();
                func(x, a, 1, 2);
                return 0;
            }
        )");
        ASSERT_EQUALS("[test.cpp:14]: (warning) Invocable \"f1\" shall not be an argument to \"func\" function/method/constructor. Only literals or variables are allowed.\n"
                      "[test.cpp:15]: (warning) Invocable \"f2\" shall not be an argument to \"func\" function/method/constructor. Only literals or variables are allowed.\n"
                      "[test.cpp:15]: (warning) Invocable \"f2\" shall not be an argument to \"X\" function/method/constructor. Only literals or variables are allowed.\n"
                      "[test.cpp:16]: (warning) Invocable \"f1\" shall not be an argument to \"func\" function/method/constructor. Only literals or variables are allowed.\n"
                      "[test.cpp:16]: (warning) Invocable \"f2\" shall not be an argument to \"func\" function/method/constructor. Only literals or variables are allowed.\n",
                      errout.str());
    }

    void constructorCall()
    {
        check(R"(
            int getInteger() {
                return 0;
            }

            class X {
                int x;
                int y{getInteger()};
                public:
                    X(int _x) : x{_x} {}
            };

            namespace SomeNamespace
            {
                static X globalX1{0};
                static X globalX2{getInteger()};
            }

            int main() {
                int v = getInteger();
                X x1{0};
                X x2{getInteger()};
                X x3{v};
                X x5 = X{0};
                X x4 = X{getInteger()};
                return 0;
            }
        )");
        ASSERT_EQUALS("[test.cpp:22]: (warning) Invocable \"getInteger\" shall not be an argument to \"x2\" function/method/constructor. Only literals or variables are allowed.\n"
                      "[test.cpp:25]: (warning) Invocable \"getInteger\" shall not be an argument to \"X\" function/method/constructor. Only literals or variables are allowed.\n"
                      "[test.cpp:8]: (warning) Invocable \"getInteger\" shall not be an argument to \"y\" function/method/constructor. Only literals or variables are allowed.\n"
                      "[test.cpp:16]: (warning) Invocable \"getInteger\" shall not be an argument to \"globalX2\" function/method/constructor. Only literals or variables are allowed.\n", errout.str());
    }

    void methodCall()
    {
        check(R"(
            class X {
                int x{0};
                public:
                    void setX(int value) { x = value; }
            };

            int getInteger() {
                return 0;
            }

            int main() {
                X x;
                int v = getInteger();
                x.setX(0);
                x.setX(getInteger());
                x.setX(v);
                return 0;
            }
        )");
        ASSERT_EQUALS("[test.cpp:16]: (warning) Invocable \"getInteger\" shall not be an argument to \"setX\" function/method/constructor. Only literals or variables are allowed.\n",
                      errout.str());
    }
};

REGISTER_TEST(TestInvocableAsFunctionArg)