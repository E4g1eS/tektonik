module;
#include "std.hpp"
#include "sdl-wrapper.hpp"
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

std::vector<TestData>& GetTestsVector()
{
    static std::vector<TestData> globalVector;
    return globalVector;
}

#ifndef DONT_COMPILE_TESTS
#define ADD_TEST_FUNC(funcName)                                         \
    void funcName();                                               \
    static bool init_##funcName = []()                             \
    {                                                              \
        GetTestsVector().push_back(TestData{funcName, #funcName}); \
        return true;                                               \
    }();                                                           \
    void funcName()
#else
#define ADD_TEST_FUNC(funcName) void funcName()
#endif

ADD_TEST_FUNC(TestTheTest)
{
    TestAssert(true, "This should never fail.");
}

ADD_TEST_FUNC(TestSparseSetSimple)
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

ADD_TEST_FUNC(TestSparseSetModifyingLastElement)
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

ADD_TEST_FUNC(TestComponentManager)
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

ADD_TEST_FUNC(TestWorld)
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

ADD_TEST_FUNC(TestTiable)
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

ADD_TEST_FUNC(TestSdl)
{
    TestAssert(SDL_Init(SDL_INIT_VIDEO), std::format("SDL_Init failed with: ''", SDL_GetError()));

    SDL_Window* window = SDL_CreateWindow("SDL3 Minimal Example", 800, 600, SDL_WINDOW_VULKAN);
    TestAssert(window, std::format("SDL_CreateWindow failed with: ''", SDL_GetError()));

    SDL_Event event;
    constexpr int kRunForThisManyTicks = 100;
    for (int i = 0; i < kRunForThisManyTicks; ++i)
        SDL_PollEvent(&event);

    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool RunAll()
{
    auto& tests = GetTestsVector();
    if (tests.empty())
        return true;

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
