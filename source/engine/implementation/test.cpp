module;
#include "std.hpp"
module test;

import sparse_set;
import ecs;

namespace tektonik::test
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

void TestComponentManager()
{
    struct NameComponent
    {
        std::string name;
    };

    struct ValueComponent
    {
        uint32_t value;
    };

    ecs::ComponentManager<NameComponent, ValueComponent> componentManager{};
    static_assert(componentManager.kComponentTypeCount == 2);
    componentManager.AddComponent(5, ValueComponent{.value = 5});
    componentManager.RemoveComponent<ValueComponent>(5);
}

void TestWorld()
{
    using namespace ecs;

    struct NameComponent
    {
        std::string name;
    };

    struct ValueComponent
    {
        uint32_t value;
    };

    World<ecs::ComponentManager<NameComponent, ValueComponent>> world{};

    Entity entity = world.NewEntity();
    world.GetComponentManager().AddComponent(entity, NameComponent{"random"});
    world.DeleteEntity(entity);

    Entity car1 = world.NewEntity();
    Entity car2 = world.NewEntity();
    world.GetComponentManager().AddComponent(car1, ValueComponent{0});
    world.GetComponentManager().AddComponent(car2, ValueComponent{1});

    auto entityRange = world.GetComponentManager().GetEntitiesWithComponents<ValueComponent>();
    for (auto entity : entityRange)
    {
        ValueComponent& valueComponent = world.GetComponentManager().GetComponent<ValueComponent>(entity);
        TestAssert(valueComponent.value == 0 || valueComponent.value == 1);
    }
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
        {             TestComponentManager,       "Test component manager functionality"},
        {                        TestWorld,                             "Test ECS world"},
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
