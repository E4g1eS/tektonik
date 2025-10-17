module;
#include "std.hpp"
export module util;

// Utility functions and classes that do not fit anywhere else.

import concepts;

export template <typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::tuple<Ts...>& tuple)
{
    constexpr std::size_t elementCount = sizeof...(Ts);
    os << "(";
    std::apply(
        [&os, &elementCount](const Ts&... tupleElements)
        {
            std::size_t index = 0;
            (
                [&os, &elementCount, &index](const auto& tupleElement)
                {
                    if constexpr (tektonik::concepts::Tiable<decltype(tupleElement)>)
                        os << ToString(tupleElement.Tie());
                    else
                        os << tupleElement << (++index != elementCount ? ", " : "");
                }(tupleElements),
                ...);
        },
        tuple);
    os << ")";
    return os;
}

export std::ostream& operator<<(std::ostream& os, const tektonik::concepts::Tiable auto& tiable)
{
    return os << tiable.Tie();
}

namespace tektonik::util
{

namespace string
{

export std::string_view Trim(const std::string_view& str, const std::locale& locale = std::locale::classic());

export std::vector<std::string_view> ParseCommandLineArgumentsToVector(int argc, const char* argv[]);

export std::unordered_map<std::string_view, std::string_view> ParseCommandLineArgumentsToMap(int argc, const char* argv[]);

export std::string ToString(const std::ranges::range auto& stringRange)
{
    std::stringstream ss;
    for (const auto& string : stringRange)
        ss << string << "\n";

    return ss.str();
}

template <typename... Ts>
std::string ToString(const std::tuple<Ts...>& tuple)
{
    std::stringstream ss;
    ss << tuple;
    return ss.str();
}

export std::string ToString(const concepts::Tiable auto& tiable)
{
    std::stringstream ss;
    ss << tiable;
    return ss.str();
}

export template <typename KeyType, typename ValueType>
std::string ToString(const std::unordered_map<KeyType, ValueType>& map)
{
    std::stringstream ss;
    for (const auto& [key, value] : map)
        ss << key << " = " << value << "\n";

    return ss.str();
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
