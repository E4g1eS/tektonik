module;
#include "std.hpp"
export module singleton;

namespace tektonik
{

template <typename T>
concept Singletonable = std::is_constructible_v<T>;

// A Singleton implementation that must be explicitly initialized and destroyed.
export template <Singletonable ContainedType>
class Singleton
{
  public:
    // Only initializes if not already initialized.
    template <typename... Args>
    static void Init(Args&&... args)
    {
        if (!GetContained().has_value())
            GetContained().emplace(std::forward<Args>(args)...);
    }

    static ContainedType& Get()
    {
        ASSUMERT(GetContained().has_value());
        return *(GetContained());
    }

    static void Destroy()
    {
        ASSUMERT(GetContained().has_value());
        GetContained() = std::nullopt;
    }

  private:
    static auto& GetContained()
    {
        static std::optional<ContainedType> contained{};
        return contained;
    }
};

}  // namespace tektonik
