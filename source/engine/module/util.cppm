module;
#include "std.hpp"
export module util;

// Utility functions and classes that do not fit anywhere else.

import concepts;

template <typename T>
concept FormattableButNotOutStreamable = std::formattable<T, char> && !requires(T obj, std::ostream& os) {
    { os << obj } -> std::same_as<std::ostream&>;
};

export std::ostream& operator<<(std::ostream& os, const FormattableButNotOutStreamable auto& onlyFormattable)
{
    std::format_to(std::ostream_iterator<char>(os), "{}", onlyFormattable);
    return os;
}

export std::ostream& operator<<(std::ostream& os, const tektonik::concepts::Tiable auto& tiable)
{
    std::format_to(std::ostream_iterator<char>(os), "{}", tiable.Tie());
    return os;
}

namespace tektonik::util
{

namespace ranges
{

export template <concepts::Pointer ValuePointer, typename TargetType = std::remove_pointer_t<ValuePointer>>
auto MakeVector(ValuePointer pointer, size_t count)
{
    std::vector<TargetType> vec;
    vec.reserve(count);
    for (size_t i = 0; i < count; ++i)
        vec.push_back(static_cast<TargetType>(pointer[i]));

    return vec;
}

}  // namespace ranges

namespace string
{

export std::string_view Trim(const std::string_view& str, const std::locale& locale = std::locale::classic());

export std::vector<std::string_view> ParseCommandLineArgumentsToVector(int argc, char* argv[]);

export std::unordered_map<std::string_view, std::string_view> ParseCommandLineArgumentsToMap(int argc, char* argv[]);

export std::string ToString(const concepts::Tiable auto& tiable)
{
    std::stringstream ss;
    ss << tiable;
    return ss.str();
}

export std::string ToString(const std::formattable auto& formattable)
{
    return std::format("{}", formattable);
}

export enum class Case {
    Lower,
    Upper,
};

template <Case caseConvert>
std::string ToCaseFromStringView(const std::string_view& str)
{
    std::stringstream ss;
    for (char c : str)
    {
        if constexpr (caseConvert == Case::Lower)
            ss << static_cast<char>(std::tolower(c));
        else
            ss << static_cast<char>(std::toupper(c));
    }

    return ss.str();
}

export template <Case caseConvert>
std::string ToCase(const concepts::StaticCastableTo<std::string_view> auto& str)
{
    if constexpr (std::is_same_v<decltype(str), const std::string_view&>)
        return ToCaseFromStringView<caseConvert>(str);
    else
        return ToCaseFromStringView<caseConvert>(static_cast<std::string_view>(str));
}

}  // namespace string

}  // namespace tektonik::util
