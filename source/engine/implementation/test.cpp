module;
#include "std.hpp"
#include "sdl-wrapper.hpp"
module test;

import sparse_set;
import ecs;
import singleton;
import logger;
import util;
import string_enum;
import std;

namespace tektonik::test
{

class TestError : public std::logic_error
{
  public:
    TestError(const std::string& message) : std::logic_error(message) {}
};

void TestAssert(
    bool condition,
    const std::string& errorMessage = "Unspecified error",
    const std::source_location& location = std::source_location::current())
{
    if (!condition)
        throw TestError(std::format("Assert failed: '{}' on line {}.", errorMessage, location.line()));
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

ADD_TEST_FUNC(TestStringEnum)
{
    using AnimalType = StringEnum<"cat", "dog", "frog">;

    AnimalType animal1{"dog"};

    TestAssert(static_cast<int>(animal1) == 1, "animal1 'dog' should map to 1.");

    switch (animal1)
    {
        case AnimalType("dog"):
            TestAssert(true, "animal1 is a dog");
            break;
        default:
            TestAssert(false, "animal1 should be a dog");
            break;
    }
}

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

    std::stringstream ss2;
    ss2 << obj1.Tie();
    std::string str2 = ss2.str();
    TestAssert(!str2.empty());
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
}

ADD_TEST_FUNC(TestStringTrim)
{
    std::string str1 = "   Hello, World!   ";
    std::string str2 = "Hello, World!";
    auto trimmed1 = util::string::Trim(str1);
    TestAssert(trimmed1 == str2, "String trimming failed.");
    std::string str3 = "\n\t  Trim me! \t\n";
    std::string str4 = "Trim me!";
    auto trimmed2 = util::string::Trim(str3);
    TestAssert(trimmed2 == str4, "String trimming with newlines and tabs failed.");
    std::string str5 = "      ";
    std::string str6 = "";
    auto trimmed3 = util::string::Trim(str5);
    TestAssert(trimmed3 == str6, "String trimming of all-whitespace string failed.");
    std::string str7 = "NoTrimNeeded";
    auto trimmed4 = util::string::Trim(str7);
    TestAssert(trimmed4 == str7, "String trimming altered a string that needed no trimming.");
    std::string str8 = "  Only prefix";
    std::string str9 = "Only prefix";
    auto trimmed5 = util::string::Trim(str8);
    TestAssert(trimmed5 == str9, "String trimming of prefix-only whitespace failed.");
    std::string str10 = "Suffix only   ";
    std::string str11 = "Suffix only";
    auto trimmed6 = util::string::Trim(str10);
    TestAssert(trimmed6 == str11, "String trimming of suffix-only whitespace failed.");
}

ADD_TEST_FUNC(TestCommandLineParsing)
{
    const char* argv[] = {
        "program",
        "--option1=value1",
        "-o2=value2",
        "--flag",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    auto argsVector = util::string::ParseCommandLineArgumentsToVector(argc, const_cast<char**>(argv));
    TestAssert(argsVector.size() == 3, "Argument vector size mismatch.");
    TestAssert(argsVector[0] == "--option1=value1", "Argument vector parsing failed for option1.");
    TestAssert(argsVector[1] == "-o2=value2", "Argument vector parsing failed for option2.");
    TestAssert(argsVector[2] == "--flag", "Argument vector parsing failed for flag.");
    auto argsMap = util::string::ParseCommandLineArgumentsToMap(argc, const_cast<char**>(argv));
    TestAssert(argsMap.size() == 3, "Argument map size mismatch.");
    TestAssert(argsMap["option1"] == "value1", "Argument map parsing failed for option1.");
    TestAssert(argsMap["o2"] == "value2", "Argument map parsing failed for option2.");
    TestAssert(argsMap["flag"] == "", "Argument map parsing failed for flag.");

    std::string toString = std::format("Parsed arguments: {}", argsMap);
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
