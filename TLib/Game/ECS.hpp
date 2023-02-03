#pragma once

#include <entt/entt.hpp>

using ECS        = entt::registry;
using Entity     = entt::entity;
auto  NullEntity = entt::null;

Entity copyEntity(ECS& ecs, Entity e)
{
    Entity r = ecs.create();
    for (auto&& curr: ecs.storage())
    {
        if (auto& storage = curr.second; storage.contains(e))
        { storage.emplace(r, storage.get(e)); }
    }
    return r;
}