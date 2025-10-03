#include "std.hpp"
import tektonik;

namespace tk = tektonik;

int main(int argc, char* argv[])
{
    using S_Logger = tk::Singleton<tk::Logger>;
    S_Logger::Init();
    S_Logger::Get().Log("Logger initialized.");

    S_Logger::Get().Log(tk::util::string::ToString(tk::util::ParseCommandLineArguments(argc, argv)));

    tk::Runtime::Test();
    tk::Runtime::OpenDemoWindow();

    S_Logger::Get().Log("Shutting down.");
    S_Logger::Destroy();

    return 0;
}
