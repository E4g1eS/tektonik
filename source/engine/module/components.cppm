module;
#include "std.hpp"
export module components;

import glm;
import ecs;

namespace components
{

export struct Transform2D
{
    glm::vec2 position = glm::gtc::zero<glm::vec2>();
    float rotation = 0;
    glm::vec2 scale = glm::gtc::one<glm::vec2>();
};
static_assert(ecs::Component<Transform2D>);

export struct Box2D
{
    glm::vec2 size = glm::gtc::one<glm::vec2>();
};
static_assert(ecs::Component<Box2D>);

export struct Circle2D
{
    float radius = 1.0f;
};
static_assert(ecs::Component<Circle2D>);

export struct Color
{
    glm::vec4 color = glm::vec4(1.0, 0.0f, 0.0f, 1.0f);
};
static_assert(ecs::Component<Color>);

export struct Sprite
{
    std::string path = "";
};
static_assert(ecs::Component<Sprite>);

export template <ecs::Component ComponentType>
struct ComponentCollection
{
    std::vector<ComponentType> collection;
};
static_assert(ecs::Component<ComponentCollection<ecs::DummyComponent>>);

}  // namespace components
