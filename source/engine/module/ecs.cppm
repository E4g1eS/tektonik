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
    Entity NewEntity()
    {
        if (unusedEntities.empty())
        {
            return nextMaxCreatedEntity++;
        }
        else
        {
            Entity unusedEntity = unusedEntities.back();
            unusedEntities.pop_back();
            return unusedEntity;
        }
    }

    void DeleteEntity(Entity entity)
    {
        assert(entity < nextMaxCreatedEntity);
        unusedEntities.push_back(entity);
    }

  private:
    // Already once created entities that were deleted.
    std::vector<Entity> unusedEntities{};
    Entity nextMaxCreatedEntity = 0;
};

template <typename T>
concept Component = std::is_default_constructible_v<T>;

template <Component... ComponentTypes>
class ComponentManager
{
  public:
    static constexpr size_t kComponentTypeCount = sizeof...(ComponentTypes);

    using ComponentSignature = std::bitset<kComponentTypeCount>;

    ComponentManager() { (InitComponent<ComponentTypes>(), ...); }

    template <Component ComponentType>
    void AddComponent(Entity entity, ComponentType&& component)
    {
        // Simply add to the array.
        GetComponentArray<ComponentType>().Add(entity, std::move(component));
        // Get current signature.
        ComponentSignature& signature = GetEntityComponentSignature(entity);
        // Remove from current entity set if currently had any components.
        if (signature != ComponentSignature{})
        {
            assert(componentsHaveEntities.contains(signature));
            assert(componentsHaveEntities[signature].contains(entity));
            componentsHaveEntities[signature].erase(entity);
        }
        // Get the bit of the component.
        size_t componentBit = ComponentTypeToBit<ComponentType>();
        assert(componentBit < signature.size());
        // Adjust the signature.
        signature[componentBit] = true;
        // Add to the adjusted entity set.
        componentsHaveEntities[signature].insert(entity);
    }

    template <Component ComponentType>
    void RemoveComponent(Entity entity)
    {
        // Simply remove from the array.
        GetComponentArray<ComponentType>().Remove(entity);
        // Get current signature.
        ComponentSignature& signature = GetEntityComponentSignature(entity);
        // Remove from current entity set.
        assert(componentsHaveEntities.contains(signature));
        assert(componentsHaveEntities[signature].contains(entity));
        componentsHaveEntities[signature].erase(entity);
        // Get the bit of the component.
        size_t componentBit = ComponentTypeToBit<ComponentType>();
        assert(componentBit < signature.size());
        // Adjust the signature.
        signature[componentBit] = false;
        // Add to the adjusted entity set if not zero.
        if (signature != ComponentSignature{})
            componentsHaveEntities[signature].insert(entity);
    }

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
    void InitComponent()
    {
        auto typeIndex = std::type_index(typeid(ComponentType));
        assert(!components.contains(typeIndex) && "Components must be unique.");
        auto derived = std::make_unique<DerivedComponentArray<ComponentType>>();
        auto base = static_cast<IComponentArray*>(derived.release());
        components[typeIndex] = std::unique_ptr<IComponentArray>(base);
    }

    template <Component ComponentType>
    auto& GetComponentArray()
    {
        auto typeIndex = std::type_index(typeid(ComponentType));
        assert(components.contains(typeIndex));
        return *static_cast<DerivedComponentArray<ComponentType>*>(components[typeIndex].get());
    }

    ComponentSignature& GetEntityComponentSignature(Entity entity)
    {
        if (entity >= entitiesHaveComponents.size())
            entitiesHaveComponents.resize(entity + 1);

        return entitiesHaveComponents[entity];
    }

    template <Component ComponentType>
    consteval size_t ComponentTypeToBit()
    {
        size_t result = std::numeric_limits<size_t>::max();
        size_t componentIndex = 0;

        auto kGetResult = [&](Component auto dummy)
        {
            if constexpr (std::is_same_v<decltype(dummy), ComponentType>)
                result = componentIndex;

            ++componentIndex;
        };

        (kGetResult(ComponentTypes{}), ...);
        return result;
    }

    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> components;
    // Tracks component signature to entities.
    std::unordered_map<ComponentSignature, std::set<Entity>> componentsHaveEntities;
    // Tracks what components a specific entity has.
    std::vector<ComponentSignature> entitiesHaveComponents;
};

template <typename T>
concept SpecifiedComponentManager = requires(T object) {
    { ComponentManager{object} } -> std::same_as<T>;
};

class SystemManager
{
};

template <SpecifiedComponentManager ComponentManagerType>
class World
{
  public:
    Entity NewEntity() { return entityManager.NewEntity(); }

  private:
    EntityManager entityManager{};
    ComponentManagerType componentManager{};
    SystemManager systemManager{};
};

};  // namespace ecs
