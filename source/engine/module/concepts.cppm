module;
#include <std.hpp>
export module concepts;

namespace concepts
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

}