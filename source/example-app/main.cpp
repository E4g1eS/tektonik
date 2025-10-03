import tektonik;

int main()
{
    using S_Logger = tektonik::Singleton<tektonik::Logger>;
    S_Logger::Init();
    S_Logger::Get().Log("Logger initialized.");

    tektonik::Runtime::Test();

    tektonik::Runtime::OpenDemoWindow();

    S_Logger::Get().Log("Shutting down.");
    S_Logger::Destroy();

    return 0;
}
