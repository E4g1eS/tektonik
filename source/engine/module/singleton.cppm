module;
#include "common-defines.hpp"
export module singleton;

import std;

namespace tektonik
{

template <typename T>
concept Singletonable = std::is_constructible_v<T>;

// A Singleton implementation that is explicitly initialized and destroyed
// in constructor and destructor.
export template <Singletonable ContainedType>
class Singleton
{
  public:
    Singleton() { GetContained().emplace(); }
    ~Singleton()
    {
        ASSUMERT(GetContained().has_value());
        GetContained() = std::nullopt;
    }

    // For non default re-initialization.
    template <typename... Args>
    static void ReInit(Args&&... args)
    {
        GetContained().emplace(std::forward<Args>(args)...);
    }

    static ContainedType& Get()
    {
        ASSUMERT(GetContained().has_value());
        return *(GetContained());
    }

  private:
    static auto& GetContained()
    {
        static std::optional<ContainedType> contained{};
        return contained;
    }
};

}  // namespace tektonik
