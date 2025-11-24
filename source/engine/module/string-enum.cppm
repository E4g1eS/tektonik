export module string_enum;

import std;

export template <size_t Length>
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

export template <size_t N>
FixedString(const char (&)[N]) -> FixedString<N - 1>;

export template <FixedString... enumValues>
class StringEnum
{
  public:
    constexpr StringEnum() = default;
    constexpr StringEnum(const char* enumValue) : option(ToOption(enumValue)) {}

    constexpr operator int() const { return option; }

    explicit operator std::string() const { return std::string("unimplemented so far"); }

    std::vector<std::string> GetAllOptions() const
    {
        std::vector<std::string> options;
        options.reserve(sizeof...(enumValues));
        (
            [&]
            {
                options.push_back(std::string(enumValues.str));
            }(),
            ...);
        return options;
    }

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
