module;
#include "std.hpp"
module test;

import sparse_set;

namespace test
{

class TestError : public std::logic_error
{
  public:
    TestError(const std::string& message) : std::logic_error(message) {}
};

void TestSparseSet()
{
    auto sparseSet = SparseSet<std::string>(20);
}

bool RunAll()
{
    struct TestData
    {
        std::function<void(void)> func;
        std::string name;
    };

    std::vector<TestData> tests = {
        {TestSparseSet, "Test sparse set"},
    };

    bool allPassed = true;

    for (const TestData& test : tests)
    {
        try
        {
            test.func();
            std::cout << std::format("Test {} successful", test.name) << std::endl;
        }
        catch (const TestError& testError)
        {
            std::cout << std::format("Test {} failed: {}", test.name, testError.what()) << std::endl;
            allPassed = false;
        }
    }

    return allPassed;
}

}  // namespace test
