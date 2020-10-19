#include "noexceptspecifier.h"

#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

class TestNoexceptSpecifier : public TestFixture
{
public:
    TestNoexceptSpecifier() : TestFixture("TestNoexceptSpecifier") {}

private:
    Settings settings;

    void run() OVERRIDE
    {
        settings.addEnabled("warning");

        TEST_CASE(noexceptSpecialMemberFunctions);
        TEST_CASE(constAndNoexceptGetters);
        TEST_CASE(noexceptFunctions);
    }

    void check(const std::string& code)
    {
        errout.str({});

        Tokenizer tokenizer{&settings, this};
        std::istringstream  istr{code};
        tokenizer.tokenize(istr, "test.cpp");

        CheckExceptionSpecifier check;
        check.runChecks(&tokenizer, &settings, this);
    }

    void noexceptSpecialMemberFunctions()
    {
        check(R"(
                class A {
                public:
                    A() = default;
                    ~A() = default; // NOT OK
                    A(const A&) = default;
                    A(A&&) = default; // NOT OK
                    A& operator=(const A&) = default;
                    A& operator=(A&&) = default; // NOT OK
                };
                )");
        ASSERT_EQUALS("[test.cpp:5]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:7]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:9]: (warning) The function shall be specified noexcept.\n",
                      errout.str());

        check(R"(
                class A {
                public:
                    A() = default;
                    ~A() noexcept = default;
                    A(const A&) = default;
                    A(A&&) noexcept = default;
                    A& operator=(const A&) = default;
                    A& operator(A&&) noexcept = default;
                };
                )");
        ASSERT_EQUALS("", errout.str());
    }

    void constAndNoexceptGetters()
    {
        check(R"(
                struct S {
                    enum E { OK, NOK };
                };
                using i32 = int;

                class A {
                public:
                    void func() { return; }
                    S get1() { return S{}; }
                    unsigned long long int get2() { return 0U; }
                    std::uint16_t get3() { return 1; }
                    i32 get4() { return m_x; }
                    bool get5() { return true; }
                    char get6() { return 'a'; }
                    S::E get7() { return S::E::OK; }
                    const S& get8() { return m_s; }
                    const S* get9() { return &m_s; }
                private:
                    int m_x;
                    const S m_s;
                };
                )");
        ASSERT_EQUALS("[test.cpp:11]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:11]: (warning) The function shall be specified const.\n"
                      "[test.cpp:12]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:12]: (warning) The function shall be specified const.\n"
                      "[test.cpp:13]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:13]: (warning) The function shall be specified const.\n"
                      "[test.cpp:14]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:14]: (warning) The function shall be specified const.\n"
                      "[test.cpp:15]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:15]: (warning) The function shall be specified const.\n"
                      "[test.cpp:16]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:16]: (warning) The function shall be specified const.\n"
                      "[test.cpp:17]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:17]: (warning) The function shall be specified const.\n"
                      "[test.cpp:18]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:18]: (warning) The function shall be specified const.\n",
                      errout.str());
    }

    void noexceptFunctions()
    {
        check(R"(
                struct S {
                    enum E { OK, NOK };
                };
                namespace N
                {
                    S::E get1() { return S::E::NOK; }
                    int get2() { return 0; }
                }
                bool get3() { return false; }
                char get4() { return 'c'; }
                std::string get5() { return "str"; }
                S get6() { return S{}; }
                )");
        ASSERT_EQUALS("[test.cpp:7]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:8]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:10]: (warning) The function shall be specified noexcept.\n"
                      "[test.cpp:11]: (warning) The function shall be specified noexcept.\n", errout.str());
    }
};

REGISTER_TEST(TestNoexceptSpecifier)