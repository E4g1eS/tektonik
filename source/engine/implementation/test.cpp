module;
#include "std.hpp"
module test;

import sparse_set;
import ecs;
import singleton;
import logger;
import util;

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

struct TestData
{
    std::function<void(void)> func;
    std::string name;
};

std::vector<TestData>& GetGlobalVector()
{
    static std::vector<TestData> globalVector;
    return globalVector;
}

inline void AddToGlobalVector(const TestData& elem)
{
    volatile static bool initialized = [&elem]()
    {
        GetGlobalVector().push_back(elem);
        return true;
    }();
    (void)initialized;  // quiet unused variable warning
}

bool throwawayValue = true;

// Usage at global scope wrapped in macro for convenience
#define ADD_TO_GLOBAL_VECTOR(elem) throwawayValue = []() { AddToGlobalVector(elem);  return true; }();

#define ADD_FUNC(funcName)                                 \
    void funcName();                                       \
    ADD_TO_GLOBAL_VECTOR((TestData{funcName, #funcName})); \
    void funcName()

ADD_FUNC(TestTheTest)
{
    std::cout << "works" << std::endl;
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

        auto Tie() const { return std::tie(name); }
    };

    struct ValueComponent
    {
        uint32_t value;

        auto Tie() const { return std::tie(value); }
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

        auto Tie() const { return std::tie(name); }
    };

    struct ValueComponent
    {
        uint32_t value;

        auto Tie() const { return std::tie(value); }
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

void TestTiable()
{
    struct TestStruct
    {
        int a;
        float b;
        std::string c;

        auto Tie() const { return std::tie(a, b, c); }
    };
    static_assert(concepts::Tiable<TestStruct>);

    TestStruct obj1{1, 2.5f, "test"};
    TestStruct obj2{1, 2.5f, "test"};
    TestStruct obj3{2, 3.0f, "different"};
    TestAssert(obj1.Tie() == obj2.Tie(), "obj1 should be equal to obj2");
    TestAssert(obj1.Tie() != obj3.Tie(), "obj1 should not be equal to obj3");

    std::stringstream ss;
    ss << obj1;
    std::string str = ss.str();
    TestAssert(!str.empty());
}

}  // namespace to_run

bool RunAll()
{
    auto& vec = GetGlobalVector();

    using namespace to_run;

    std::vector<TestData> tests = {
        {              TestSparseSetSimple,                            "Test sparse set"},
        {TestSparseSetModifyingLastElement, "Test sparse set with touching last element"},
        {             TestComponentManager,       "Test component manager functionality"},
        {                        TestWorld,                             "Test ECS world"},
        {                       TestTiable,                        "Test tiable concept"},
    };

    bool allPassed = true;

    Singleton<Logger>::Get().Log("Running tests...");

    for (const TestData& test : tests)
    {
        try
        {
            test.func();
            Singleton<Logger>::Get().Log(std::format("Test successful '{}'.", test.name));
        }
        catch (const TestError& testError)
        {
            Singleton<Logger>::Get().Log<LogLevel::Error>(std::format("Test failed '{}' with: {}", test.name, testError.what()));
            allPassed = false;
        }
    }

    return allPassed;
}

}  // namespace tektonik::test
