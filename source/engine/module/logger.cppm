module;
#include "std.hpp"
export module logger;

import common;
import singleton;
import concepts;
import util;

namespace tektonik
{

export enum class LogLevel { Error, Warning, Info, Debug, Empty };

template <typename T>
concept Loggable = concepts::OutStreamable<T>;

export class Logger
{
  public:
    Logger(std::ostream& outputStream = std::cout) noexcept : outputStream(outputStream) {}

    template <LogLevel level = LogLevel::Info, std::size_t N>
    void Log(const char (&message)[N])
    {
        LogLevel<level>();
        std::operator<<(outputStream, message);
        std::cout << std::endl;
    }
    
    template <LogLevel level = LogLevel::Info>
    void Log(const Loggable auto& message)
    {
        LogLevel<level>();
        outputStream << message << std::endl;
    }

  private:
    template <LogLevel level>
    void LogLevel()
    {
        if constexpr (level == LogLevel::Error)
            outputStream << "[ERROR] ";
        else if constexpr (level == LogLevel::Warning)
            outputStream << "[WARNING] ";
        else if constexpr (level == LogLevel::Info)
            outputStream << "[INFO] ";
        else if constexpr (level == LogLevel::Debug && common::kDebugBuild)
            outputStream << "[DEBUG] ";
    }

    std::ostream& outputStream;
};

}  // namespace tektonik
