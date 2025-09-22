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

void TestAssert(bool condition, const std::string& errorMessage = "Unspecified error")
{
    if (!condition)
        throw TestError(errorMessage);
}

// Here are all the tests that should be run.
namespace to_run
{

void TestSparseSetSimple()
{
    auto sparseSet = SparseSet<std::string>(10);

    auto checkValidity = [&]()
    {
        if (!sparseSet.IsValid())
            throw TestError("Sparse set is not valid.");
    };

    sparseSet.Add(4, "First element");
    TestAssert(sparseSet.Contains(4));
    TestAssert(!sparseSet.Contains(8));
    checkValidity();
    sparseSet.Add(8, "Second element");
    TestAssert(sparseSet.Contains(4));
    TestAssert(sparseSet.Contains(8));
    checkValidity();
    sparseSet.Remove(4);
    checkValidity();
    TestAssert(sparseSet.Contains(8));
    TestAssert(!sparseSet.Contains(4));
}

void TestSparseSetModifyingLastElement()
{
    auto sparseSet = SparseSet<std::string>(10);

    auto checkValidity = [&]()
    {
        if (!sparseSet.IsValid())
            throw TestError("Sparse set is not valid.");
    };

    sparseSet.Add(4, "First element");
    TestAssert(sparseSet.Contains(8) == false);
    checkValidity();
    sparseSet.Add(8, "Second element");
    checkValidity();
    sparseSet.Remove(8);
    checkValidity();
    sparseSet.Remove(4);
    checkValidity();
}

}  // namespace to_run

bool RunAll()
{
    struct TestData
    {
        std::function<void(void)> func;
        std::string name;
    };

    using namespace to_run;

    std::vector<TestData> tests = {
        {              TestSparseSetSimple,                            "Test sparse set"},
        {TestSparseSetModifyingLastElement, "Test sparse set with touching last element"},
    };

    bool allPassed = true;

    for (const TestData& test : tests)
    {
        try
        {
            test.func();
            std::cout << std::format("Test successful '{}'.", test.name) << std::endl;
        }
        catch (const TestError& testError)
        {
            std::cout << std::format("Test failed '{}' with: {}", test.name, testError.what()) << std::endl;
            allPassed = false;
        }
    }

    return allPassed;
}

}  // namespace test
