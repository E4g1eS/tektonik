module;
#include "common-defines.hpp"
module util;

import singleton;
import logger;
import std;
import assert;

namespace tektonik::util
{

namespace string
{

std::string_view Trim(const std::string_view& str, const std::locale& locale)
{
    auto kIsSpace = [&](char ch) { return std::isspace(ch, locale); };

    auto firstCharIt = std::ranges::find_if_not(str, kIsSpace);
    if (firstCharIt == str.end())
        return "";

    auto lastCharIt = std::ranges::find_if_not(std::views::reverse(str), kIsSpace);

    size_t offset = std::distance(str.begin(), firstCharIt);
    size_t count = std::distance(firstCharIt, lastCharIt.base());

    return str.substr(offset, count);
}

std::vector<std::string_view> ParseCommandLineArgumentsToVector(int argc, char* argv[])
{
    ASSUMERT(argc >= 1);

    std::vector<std::string_view> args;
    for (int i = 1; i < argc; ++i)
        args.push_back(argv[i]);

    return args;
}

std::unordered_map<std::string_view, std::string_view> ParseCommandLineArgumentsToMap(int argc, char* argv[])
{
    std::vector<std::string_view> argsVector = ParseCommandLineArgumentsToVector(argc, argv);
    std::unordered_map<std::string_view, std::string_view> argsMap;
    std::ranges::for_each(
        argsVector,
        [&argsMap](std::string_view arg)
        {
            if (arg.empty() || arg[0] != '-')
            {
                Singleton<Logger>::Get().Log<LogLevel::Warning>(
                    std::format("Invalid command line argument: '{}'. All arguments must start with '-'. The argument will be skipped.", arg));
                return;
            }

            arg.remove_prefix(std::min(arg.find_first_not_of('-'), arg.size()));
            auto equals = arg.find('=');
            if (equals != std::string_view::npos)
                argsMap[arg.substr(0, equals)] = arg.substr(equals + 1);
            else
                argsMap[arg] = "";
        });

    return argsMap;
}

}  // namespace string

}  // namespace tektonik::util
