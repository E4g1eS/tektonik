export module assert;

import std;

namespace tektonik::assert
{

export class AssertError : public std::logic_error
{
  public:
    AssertError() : std::logic_error("Assert failed.") {}
};

export void Assert(bool condition)
{
    if (!condition)
        throw AssertError();
}

}  // namespace tektonik::assert
