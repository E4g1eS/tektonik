module;
#include <std.hpp>
export module concepts;

namespace tektonik::concepts
{

template <typename T, template <typename...> class Template>
struct is_instantiation_of : std::false_type
{
};

template <template <typename...> class Template, typename... Args>
struct is_instantiation_of<Template<Args...>, Template> : std::true_type
{
};

template <typename T, template <typename...> class Template>
constexpr bool is_instantiation_of_v = is_instantiation_of<T, Template>::value;

export template <typename T, template <typename...> class Template>
concept InstantiatedFrom = is_instantiation_of_v<T, Template>;

export template <typename From, typename To>
concept StaticCastableTo = requires(From&& from) { static_cast<To>(from); };

export template <typename T>
concept StringLike = StaticCastableTo<T, std::string> && requires(T x) {
    { x[0] } -> std::convertible_to<char>;
};
static_assert(StringLike<std::string>);
static_assert(StringLike<std::string_view>);
static_assert(StringLike<char*>);

export template <typename T>
concept LoadableFromStringView = requires(T a, const std::string_view& str) {
    { a.LoadFromStringView(str) } -> std::same_as<void>;
};

}  // namespace tektonik::concepts
