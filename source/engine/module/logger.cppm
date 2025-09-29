module;
#include "std.hpp"
export module logger;

import common;

namespace logging
{

export enum class LogLevel { Error, Warning, Info, Debug };

template <typename T>
concept Loggable = requires(T obj) { std::cout << obj << std::endl; };

export class Logger
{
  public:
    Logger(std::ostream& outputStream = std::cout) : outputStream(outputStream) {}

    template <LogLevel level>
    void Log(const Loggable auto& message)
    {
        if constexpr (level == LogLevel::Error)
            outputStream << "[ERROR]" << message << std::endl;
        else if constexpr (level == LogLevel::Warning)
            outputStream << "[WARNING]" << message << std::endl;
        else if constexpr (level == LogLevel::Info)
            outputStream << "[INFO]" << message << std::endl;
        else if constexpr (level == LogLevel::Debug && common::kDebugBuild)
            outputStream << "[DEBUG]" << message << std::endl;
    }

  private:
    std::ostream& outputStream;
};

}  // namespace log
