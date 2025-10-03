module;
#include "std.hpp"
export module config;

import singleton;
import concepts;

namespace tektonik::config
{

export class Manager
{
  public:
  private:
};

template <typename T>
class Variable
{
  public:
    Variable() = default;
    Variable(const concepts::StringLike auto& name, const T& defaultValue) : name(name), value(defaultValue) {}

    T& operator*() { return value; }
    const T& operator*() const { return value; }

  private:
    std::string name = "";
    T value = T();
};

export using Int32 = Variable<int32_t>;
export using UInt32 = Variable<uint32_t>;
export using Float = Variable<float>;
export using String = Variable<std::string>;
export using Bool = Variable<bool>;

}  // namespace tektonik::config
