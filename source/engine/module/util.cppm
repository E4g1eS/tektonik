module;
#include "std.hpp"
export module util;

import concepts;

// Utility functions and classes that do not fit anywhere else.
namespace tektonik::util
{

export std::vector<std::string_view> ParseCommandLineArgumentsToVector(int argc, char* argv[])
{
    ASSUMERT(argc >= 1);

    std::vector<std::string_view> args;
    for (int i = 1; i < argc; ++i)
        args.push_back(argv[i]);

    return args;
}

export std::unordered_map<std::string_view, std::string_view> ParseCommandLineArgumentsToMap(int argc, char* argv[])
{
    std::vector<std::string_view> argsVector = ParseCommandLineArgumentsToVector(argc, argv);
    std::unordered_map<std::string_view, std::string_view> argsMap;
    std::ranges::for_each(
        argsVector,
        [&argsMap](std::string_view arg)
        {
            arg.remove_prefix(std::min(arg.find_first_not_of('-'), arg.size()));
            auto equals = arg.find('=');
            if (equals != std::string_view::npos)
                argsMap[arg.substr(0, equals)] = arg.substr(equals + 1);
            else
                argsMap[arg] = "";
        });

    return argsMap;
}

namespace string
{

export std::string ToString(const std::ranges::range auto& stringRange)
{
    std::stringstream ss;
    for (const auto& string : stringRange)
        ss << string << "\n";

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
    if constexpr (std::is_same_v < decltype(str), const std::string_view&>)
        return ToCaseFromStringView<caseConvert>(str);
    else
        return ToCaseFromStringView<caseConvert>(static_cast<std::string_view>(str));
}



}  // namespace string

}  // namespace tektonik::util
