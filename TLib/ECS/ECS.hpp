
#pragma once

#define flecs_STATIC
#include <flecs.h>

using Entity = flecs::entity;
using ECS    = flecs::world;
using System = flecs::system;

Entity createChild(Entity parent)
{
    return parent.world().entity().child_of(parent);
}

template <typename T, typename... Args>
T& emplaceComponent(Entity entity, Args&&... args)
{
    entity.emplace<T>(args...);
    return *entity.get_mut<T>();
}
