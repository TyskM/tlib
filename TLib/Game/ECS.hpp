#pragma once

#include <entt/entt.hpp>

using ECS        = entt::registry;
using Entity     = entt::entity;
auto  NullEntity = entt::null;
using entt::exclude_t;

Entity copyEntity(ECS& ecs, Entity e)
{
    Entity copy = ecs.create();
    for (auto&& curr: ecs.storage())
    {
        if (auto& storage = curr.second; storage.contains(e))
        { storage.push(copy, storage.value(e)); }
    }
    return copy;
}

Entity copyEntity(ECS& src, ECS& dst, Entity entity)
{
    Entity copy = dst.create();

    for (auto [id, storage] : src.storage())
    {
        auto* other = dst.storage(id);
        // Entt claims to not require registering components, but it's needed for copying between registries. hmmmmmmmmmmmm....
        ASSERTMSG(other, "Tried to copy an unregistered component. Use registerComponent()")
        if (other && storage.contains(entity))
        { other->push(copy, storage.value(entity)); }
    }

    return copy;
}

template <typename T>
void registerComponent(ECS& ecs)
{
    ecs.storage<T>();
}

template <typename... Types>
void registerComponents(ECS& ecs)
{
    (registerComponent<Types>(ecs), ...);
}