export module assert;

import std;

namespace tektonik::assertion
{
class AssertionError : public std::runtime_error
{
  public:
    AssertionError() : std::runtime_error("Assert is false.") {}
};

export void Assert(const bool condition)
{
    if (!condition)
        throw AssertionError();
}

}  // namespace tektonik::assert
