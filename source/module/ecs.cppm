module;
#include "std.hpp"
export module ecs;

export namespace ecs
{

// Basically just an ID.
using Entity = uint32_t;

class Manager
{
  public:
  private:
    std::priority_queue<Entity, std::vector<Entity>, std::greater<Entity>> unusedEntities{};
};

};  // namespace ecs
