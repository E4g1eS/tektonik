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

void SomeFunction()
{
    std::string str;
}
