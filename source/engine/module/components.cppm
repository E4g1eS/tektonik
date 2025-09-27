module;
#include "std.hpp"
export module components;

import glm;

namespace components
{

export struct Transform2D
{
    glm::vec2 position = glm::gtc::zero<glm::vec2>();
    float rotation = 0;
    glm::vec2 scale = glm::gtc::one<glm::vec2>();
};

export struct Box2D
{
    glm::vec2 size = glm::gtc::one<glm::vec2>();
    glm::vec4 color = glm::vec4(1.0, 0.0f, 0.0f, 1.0f);
};

export struct Sprite
{
    std::string path = "";
};

}