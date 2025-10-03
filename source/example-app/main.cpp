#include "std.hpp"
import tektonik;

namespace tk = tektonik;

int main(int argc, char* argv[])
{
    using S_Logger = tk::Singleton<tk::Logger>;
    S_Logger::Init();
    S_Logger::Get().Log("Logger initialized.");

    auto argsMap = tk::util::ParseCommandLineArgumentsToMap(argc, argv);

    S_Logger::Get().Log(std::format("Command line arguments:\n", tk::util::string::ToString(argsMap)));

    tk::Runtime::Test();
    tk::Runtime::OpenDemoWindow();

    S_Logger::Get().Log("Shutting down.");
    S_Logger::Destroy();

    return 0;
}
