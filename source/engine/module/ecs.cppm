module;
#include "std.hpp"
export module ecs;

import sparse_set;
import concepts;

export namespace tektonik::ecs
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
        ASSUMERT(entity < nextMaxCreatedEntity);
        unusedEntities.push_back(entity);
    }

  private:
    // Already once created entities that were deleted.
    std::vector<Entity> unusedEntities{};
    Entity nextMaxCreatedEntity = 0;
};

class EntityRange
{
  public:
    using InputRange = std::vector<std::set<Entity>*>;

    EntityRange(InputRange&& entitySets)
        : entitySets(std::move(entitySets)), view(std::views::join(std::views::transform(this->entitySets, TransformFunc)))
    {
    }

    auto begin() { return view.begin(); }
    auto end() { return view.end(); }

  private:
    static std::set<Entity>& TransformFunc(std::set<Entity>* entitySet) { return *entitySet; }

    InputRange entitySets;
    decltype(std::views::join(std::views::transform(entitySets, TransformFunc))) view;
};

template <typename T>
concept Component = std::is_default_constructible_v<T> && concepts::Tiable<T>;

struct DummyComponent
{
    auto Tie() const { return std::tie(); }
};
static_assert(Component<DummyComponent>);

template <Component... ComponentTypes>
class ComponentManager
{
  public:
    static constexpr size_t kComponentTypeCount = sizeof...(ComponentTypes);

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
            ASSUMERT(signaturesHaveEntities.contains(signature));
            ASSUMERT(signaturesHaveEntities[signature].contains(entity));
            signaturesHaveEntities[signature].erase(entity);
        }
        // Get the bit of the component.
        size_t componentBit = GetComponentTypeIndex<ComponentType>();
        ASSUMERT(componentBit < signature.size());
        // Adjust the signature.
        signature[componentBit] = true;
        // Add to the adjusted entity set.
        signaturesHaveEntities[signature].insert(entity);
    }

    template <Component ComponentType>
    void RemoveComponent(Entity entity)
    {
        // Simply remove from the array.
        GetComponentArray<ComponentType>().Remove(entity);
        // Get current signature.
        ComponentSignature& signature = GetEntityComponentSignature(entity);
        // Remove from current entity set.
        ASSUMERT(signaturesHaveEntities.contains(signature));
        ASSUMERT(signaturesHaveEntities[signature].contains(entity));
        signaturesHaveEntities[signature].erase(entity);
        // Get the bit of the component.
        size_t componentBit = GetComponentTypeIndex<ComponentType>();
        ASSUMERT(componentBit < signature.size());
        // Adjust the signature.
        signature[componentBit] = false;
        // Add to the adjusted entity set if not zero.
        if (signature != ComponentSignature{})
            signaturesHaveEntities[signature].insert(entity);
    }

    template <Component ComponentType>
    Component auto& GetComponent(Entity entity)
    {
        return GetComponentArray<ComponentType>().Get(entity);
    }

    void RemoveAllComponents(Entity entity)
    {
        const ComponentSignature& signature = GetEntityComponentSignature(entity);

        // Remove entity from its component arrays.
        size_t typeIndex = 0;
        const auto removeEntityFromArray = [&]<Component ComponentType>()
        {
            if (signature[typeIndex])
                GetComponentArray<ComponentType>().Remove(entity);
            ++typeIndex;
        };
        (removeEntityFromArray.template operator()<ComponentTypes>(), ...);

        ASSUMERT(signaturesHaveEntities.contains(signature));
        ASSUMERT(signaturesHaveEntities[signature].contains(entity));
        signaturesHaveEntities[signature].erase(entity);

        entitiesHaveComponents[entity] = kNullComponentSignature;
    }

    template <Component... SelectedComponents>
    EntityRange GetEntitiesWithComponents()
    {
        std::vector<std::set<Entity>*> entitySets{};
        ComponentSignature wantedSignature = GetSignatureFromComponents<SelectedComponents...>();

        for (auto& [iteratedSignature, set] : signaturesHaveEntities)
        {
            const bool containsAllWantedBits = ((iteratedSignature & wantedSignature) == wantedSignature);
            if (containsAllWantedBits)
                entitySets.push_back(&set);
        }

        return EntityRange(std::move(entitySets));
    }

  private:
    using ComponentSignature = std::bitset<kComponentTypeCount>;
    static constexpr ComponentSignature kNullComponentSignature = ComponentSignature{};
    static constexpr size_t kInvalidTypeIndex = std::numeric_limits<size_t>::max();

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
        auto typeIndex = GetComponentTypeIndex<ComponentType>();
        ASSUMERT(!componentArrays[typeIndex] && "Components must be unique.");
        auto derived = std::make_unique<DerivedComponentArray<ComponentType>>();
        auto base = static_cast<IComponentArray*>(derived.release());
        componentArrays[typeIndex] = std::unique_ptr<IComponentArray>(base);
    }

    template <Component ComponentType>
    auto& GetComponentArray()
    {
        auto typeIndex = GetComponentTypeIndex<ComponentType>();
        return *static_cast<DerivedComponentArray<ComponentType>*>(componentArrays[typeIndex].get());
    }

    ComponentSignature& GetEntityComponentSignature(Entity entity)
    {
        if (entity >= entitiesHaveComponents.size())
            entitiesHaveComponents.resize(entity + 1);

        return entitiesHaveComponents[entity];
    }

    template <Component ComponentType>
    consteval size_t GetComponentTypeIndex(bool allowInvalid = false)
    {
        size_t result = kInvalidTypeIndex;
        size_t componentIndex = 0;

        const auto getResult = [&]<Component IteratedComponentType>()
        {
            if constexpr (std::is_same_v<IteratedComponentType, ComponentType>)
                result = componentIndex;

            ++componentIndex;
        };

        (getResult.template operator()<ComponentTypes>(), ...);
        ASSUMERT(allowInvalid || result != kInvalidTypeIndex);
        return result;
    }

    template <Component... SelectedComponentTypes>
    consteval ComponentSignature GetSignatureFromComponents()
    {
        auto signature = ComponentSignature{};
        ((signature[GetComponentTypeIndex<SelectedComponentTypes>()] = true), ...);
        return signature;
    }

    // An array of component arrays.
    std::array<std::unique_ptr<IComponentArray>, kComponentTypeCount> componentArrays;
    // Tracks component signature to entities.
    std::unordered_map<ComponentSignature, std::set<Entity>> signaturesHaveEntities;
    // Tracks what components a specific entity has.
    std::vector<ComponentSignature> entitiesHaveComponents;
};

class SystemManager
{
};

template <concepts::InstantiatedFrom<ComponentManager> ComponentManagerType>
class World
{
  public:
    Entity NewEntity() { return entityManager.NewEntity(); }
    void DeleteEntity(Entity entity)
    {
        componentManager.RemoveAllComponents(entity);
        entityManager.DeleteEntity(entity);
    }

    auto& GetComponentManager() { return componentManager; }
    const auto& GetComponentManager() const { return componentManager; }

  private:
    EntityManager entityManager{};
    ComponentManagerType componentManager{};
    SystemManager systemManager{};
};

};  // namespace ecs
