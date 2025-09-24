module;
#include "std.hpp"
export module ecs;

import sparse_set;

export namespace ecs
{

// Basically just an ID.
using Entity = uint32_t;

class EntityManager
{
  public:
  private:
    std::priority_queue<Entity, std::vector<Entity>, std::greater<Entity>> unusedEntities{};
};

template <typename ComponentType>
concept Component = std::is_default_constructible_v<ComponentType>;

template <Component... ComponentTypes>
class ComponentManager
{
  public:
    using EntitySignature = std::bitset<sizeof...(ComponentTypes)>;

    ComponentManager()
    {
        size_t nextComponentIndex = 0;
        (InitComponent<ComponentTypes>(nextComponentIndex), ...);
    }

    template <Component ComponentType>
    auto& GetComponentArray()
    {
        auto typeIndex = std::type_index(typeid(ComponentType));
        assert(components.contains(typeIndex));
        return *static_cast<DerivedComponentArray<ComponentType>*>(components[typeIndex].get());
    }

    auto size() const noexcept { return components.size(); }

  private:
    struct IComponentArray
    {
      public:
        virtual ~IComponentArray() = default;
    };

    template <Component ComponentType>
    class DerivedComponentArray : public IComponentArray, public SparseSet<ComponentType, Entity>
    {
      public:
        virtual ~DerivedComponentArray() = default;
    };

    template <Component ComponentType>
    void InitComponent(size_t& nextComponentIndex)
    {
        auto typeIndex = std::type_index(typeid(ComponentType));

        auto derived = std::make_unique<DerivedComponentArray<ComponentType>>();
        auto base = static_cast<IComponentArray*>(derived.release());
        components[typeIndex] = std::unique_ptr<IComponentArray>(base);
    }

    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> components;
};

class SystemManager
{
};

class World
{
  public:
  private:
};

};  // namespace ecs
