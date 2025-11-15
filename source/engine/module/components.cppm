module;
#include "std.hpp"
export module components;

import glm;
import ecs;
import std;

// A collection of "default" components.
namespace tektonik::components
{

export struct Transform2D
{
    glm::vec2 position = glm::gtc::zero<glm::vec2>();
    float rotation = 0;
    glm::vec2 scale = glm::gtc::one<glm::vec2>();

    auto Tie() const { return std::tie(position, rotation, scale); }
};
static_assert(ecs::Component<Transform2D>);

export struct Box2D
{
    glm::vec2 size = glm::gtc::one<glm::vec2>();

    auto Tie() const { return std::tie(size); }
};
static_assert(ecs::Component<Box2D>);

export struct Circle2D
{
    float radius = 1.0f;

    auto Tie() const { return std::tie(radius); }
};
static_assert(ecs::Component<Circle2D>);

export struct Color
{
    glm::vec4 color = glm::vec4(1.0, 0.0f, 0.0f, 1.0f);

    auto Tie() const { return std::tie(color); }
};
static_assert(ecs::Component<Color>);

export struct Sprite
{
    std::string path = "";

    auto Tie() const { return std::tie(path); }
};
static_assert(ecs::Component<Sprite>);

export template <ecs::Component ComponentType>
struct ComponentCollection
{
    std::vector<ComponentType> collection;

    auto Tie() const { return std::tie(collection); }
};
static_assert(ecs::Component<ComponentCollection<ecs::DummyComponent>>);

}  // namespace tektonik::components
