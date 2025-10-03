module;
#include "std.hpp"
export module util;

// Utility functions and classes that do not fit anywhere else.
namespace tektonik::util
{

export std::vector<std::string_view> ParseCommandLineArguments(int argc, char* argv[])
{
    ASSUMERT(argc >= 1);

    std::vector<std::string_view> args;
    for (int i = 1; i < argc; ++i)
        args.push_back(argv[i]);

    return args;
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

}

}  // namespace tektonik::util
