module;
#include "std.hpp"
export module config;

import singleton;
import concepts;
import util;
import std;

namespace tektonik::config
{

class ConfigParseError : public std::runtime_error
{
  public:
    ConfigParseError(const std::string& message) : std::runtime_error(message) {}
};

template <typename T>
class Variable
{
  public:
    Variable() = default;
    Variable(const concepts::StringLike auto& name, const T& defaultValue);
    ~Variable();

    Variable(const Variable&) = delete;
    Variable& operator=(const Variable&) = delete;

    T& operator*() { return value; }
    const T& operator*() const { return value; }

    // Specialize this for other types.
    void LoadFromStringView(const std::string_view& input) { value = input; }

  private:
    std::string name = "";
    T value = T();
};
export using String = Variable<std::string>;
export using Int32 = Variable<std::int32_t>;
export using UInt32 = Variable<std::uint32_t>;
export using Float = Variable<float>;
export using Bool = Variable<bool>;

export class Manager
{
  public:
    void RegisterVariable(const std::string& name, auto* variable)
    {
        ASSUMERT(!variables.contains(name));
        variables.insert({name, variable});
    }

    void UnregisterVariable(const std::string& name)
    {
        ASSUMERT(variables.contains(name));
        variables.erase(name);
    }

    auto& GetVariables() { return variables; }

  private:
    std::map<std::string, std::variant<String*, Int32*, UInt32*, Float*, Bool*>> variables;
};

template <typename T>
Variable<T>::Variable(const concepts::StringLike auto& name, const T& defaultValue) : name(name), value(defaultValue)
{
    Singleton<Manager>::Get().RegisterVariable(name, this);
}

template <typename T>
Variable<T>::~Variable()
{
    if (name != "")
        Singleton<Manager>::Get().UnregisterVariable(name);
}

template <>
void Variable<int32_t>::LoadFromStringView(const std::string_view& input)
{
    try
    {
        static_assert(sizeof(int32_t) <= sizeof(int));
        value = std::stoi(std::string(input));
    }
    catch (const std::exception&)
    {
        throw ConfigParseError(std::format("Could not parse '{}' as int32_t.", input));
    }
}

template <>
void Variable<uint32_t>::LoadFromStringView(const std::string_view& input)
{
    try
    {
        int32_t temp = std::stoi(std::string(input));
        if (temp < 0)
            throw ConfigParseError(std::format("Could not parse '{}' as uint32_t.", input));
        value = static_cast<uint32_t>(temp);
    }
    catch (const std::exception&)
    {
        throw ConfigParseError(std::format("Could not parse '{}' as uint32_t.", input));
    }
}

template <>
void Variable<float>::LoadFromStringView(const std::string_view& input)
{
    try
    {
        value = std::stof(std::string(input));
    }
    catch (const std::exception&)
    {
        throw ConfigParseError(std::format("Could not parse '{}' as float.", input));
    }
}

template <>
void Variable<bool>::LoadFromStringView(const std::string_view& input)
{
    std::string inputLower = util::string::ToCase<util::string::Case::Lower>(input);

    if (inputLower == "true" || inputLower == "1")
        value = true;
    else if (inputLower == "false" || inputLower == "0")
        value = false;
    else
        throw ConfigParseError(std::format("Could not parse '{}' as bool.", input));
}

}  // namespace tektonik::config
