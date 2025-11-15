module;
#include <std.hpp>
export module concepts;

import std;

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
concept LoadableFromStringView = requires(T obj, const std::string_view& str) {
    { obj.LoadFromStringView(str) } -> std::same_as<void>;
};

export template <typename T>
concept Stringable = requires(T obj) {
    { obj.ToString() } -> std::same_as<std::string>;
};

export template <typename T>
concept OutStreamable = requires(T obj, std::ostream& os) {
    { os << obj } -> std::same_as<std::ostream&>;
};

export template <typename T>
concept Tiable = requires(T obj) {
    { obj.Tie() } -> InstantiatedFrom<std::tuple>;
};

export template <typename T>
concept Pointer = std::is_pointer_v<T>;

export template <typename RangeType, typename WantedType>
concept RangeOfExactly = std::ranges::range<RangeType> && requires(RangeType range) {
    { *std::ranges::begin(range) } -> std::same_as<WantedType>;
};

export template <typename RangeType, typename WantedType>
concept RangeOfCastableTo = std::ranges::range<RangeType> && requires(RangeType range) {
    { *std::ranges::begin(range) } -> StaticCastableTo<WantedType>;
};

export template <typename RangeType, typename WantedType>
concept RangeOfConstructible = std::ranges::range<RangeType> && requires(RangeType range) {
    { WantedType(*std::ranges::begin(range)) };
};

}  // namespace tektonik::concepts
