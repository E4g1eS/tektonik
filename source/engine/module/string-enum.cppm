export module string_enum;

import std;

template <size_t Length>
struct FixedString
{
    template <size_t InputLength>
    consteval FixedString(const char (&inputStr)[InputLength])
    {
        for (size_t i = 0; i < Length; ++i)
            str[i] = inputStr[i];
    }

    constexpr bool IsSameAs(const char* other) const
    {
        for (size_t i = 0; i < Length; ++i)
            if (str[i] != other[i])
                return false;

        return true;
    }

    size_t size() const { return Length; }

    char str[Length];
};

template <size_t N>
FixedString(const char (&)[N]) -> FixedString<N - 1>;

export template <FixedString... enumValues>
class StringEnum
{
  public:
    constexpr StringEnum() = default;
    constexpr StringEnum(const char* enumValue) : option(ToOption(enumValue)) {}

    constexpr operator int() const { return option; }

    explicit operator std::string() const { return std::string("unimplemented so far"); }

  private:
    constexpr static int ToOption(const char* str)
    {
        int counter = 0;
        int option = -1;

        (
            [&]
            {
                if (enumValues.IsSameAs(str))
                    option = counter;

                ++counter;
            }(),
            ...);

        return option;
    }

    int option = 0;
};

using Animal = StringEnum<"cat", "dog", "frog">;

void Func()
{
    // ReflectiveEnum<"dog"> dogEnum;
    // std::string dogStr = dogEnum.GetStr();

    Animal animal = Animal("dog");

    switch (animal)
    {
        case Animal("dog"):
            std::cout << "animals is dog" << std::endl;
            break;
        default:
            std::cout << "animals is not zero" << std::endl;
            break;
    }
}
