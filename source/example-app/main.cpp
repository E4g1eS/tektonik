#include "std.hpp"
import tektonik;

namespace tk = tektonik;

int main(int argc, char* argv[])
{
    tk::Runtime runtime = tk::Runtime();

    using S_Logger = tk::Singleton<tk::Logger>;

    auto argsMap = tk::util::ParseCommandLineArgumentsToMap(argc, argv);

    S_Logger::Get().Log(std::format("Command line arguments:\n{}", tk::util::string::ToString(argsMap)));

    tk::Runtime::Test();
    tk::Runtime::OpenDemoWindow();

    return 0;
}
