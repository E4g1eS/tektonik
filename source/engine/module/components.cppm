module;
#include "std.hpp"
#include <glm/glm.hpp>
export module components;

namespace components
{
export struct Transform2D
{
    glm::vec2 position;
};

}