#pragma once

#include <TLib/Containers/Stack.hpp>
#include <TLib/Containers/Vector.hpp>
#include <TLib/Containers/Queue.hpp>
#include <TLib/Event.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Logging.hpp>
#include <TLib/Containers/Ref.hpp>
#include <type_traits>

struct EntityID
{
    using IDType      = uint32_t; // This has to be signed for null checking
    using VersionType = uint32_t;

    static constexpr IDType NullID = 0;

    IDType      id      = NullID;
    VersionType version = 0;

    EntityID(IDType id, VersionType version)
    {
        this->id      = id;
        this->version = version;
    }

    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(id, version);
    }

    // Default construct to a Null state. EntityID::null() -> true
    EntityID() = default;

    void nullify() { *this = EntityID(); }
    void resetVersion() { version = 0; }

    bool null()  const { return id == NullID; }
    bool valid() const { return !null(); }

    IDType      getID()      const { return id; }
    VersionType getVersion() const { return version; }

    size_t hash() const
    { return (static_cast<uint64_t>(version) << 32) | id; }

    String toString() const
    { return fmt::format("(ID: {}, Version: {})", id, version); }

    bool operator==(const EntityID& other) const { return hash() == other.hash(); }
    bool operator!=(const EntityID& other) const { return !(*this == other); }

    friend class EntityIDManager;
};

namespace eastl
{
    template <>
    struct hash<EntityID>
    {
        size_t operator()(const EntityID& p) const
        { return p.hash(); }
    };
}

class EntityIDManager
{
public:
    size_t          maxEntities = 1024;
    Stack<EntityID> availableEntityIds;
    size_t          entityCount = 0;

    EntityIDManager(size_t maxEntities = 1024)
    {
        // https://austinmorlan.com/posts/entity_component_system
        // Initialize the stack with all possible entity IDs
        this->maxEntities = maxEntities;
        for (EntityID::IDType id = maxEntities; id >= 1; --id)
        { availableEntityIds.push(EntityID(id, 0)); }
        ASSERT(availableEntityIds.size() == maxEntities);
    }

    EntityID getFreeID()
    {
        ASSERT(entityCount < maxEntities); // Too many entities

        // Take an ID from the front of the stack
        EntityID id = availableEntityIds.top();
        availableEntityIds.pop();
        ++entityCount;

        return id;
    }

    // Get an ID identical to the specified ID if possible.
    // Good for client-server situations
    // Returned EntityID.null() == true if getting the specified
    // ID is not possible (It's already taken)
    EntityID getSpecificID(EntityID id)
    {
        auto& cont = availableEntityIds.get_container();

        for (auto it = cont.rbegin(); it != cont.rend(); ++it)
        {
            auto& availId = *it;
            if (availId.id == id.id)
            {
                ASSERT(entityCount < maxEntities);
                cont.erase(it);
                ++entityCount;
                return id;
            }
        }

        ASSERT(false); // Could not get ID
        return EntityID();
    }

    void returnID(EntityID entity)
    {
        ASSERT(entity.id < maxEntities); // entity out of range
        ASSERT(entity.valid());

        --entityCount;

        // Entity version is maxed out, disable by not putting back on the available id stack
        if (entity.version == std::numeric_limits<EntityID::VersionType>::max())
        {

        }
        // Put the destroyed ID back on the stack
        else
        {
            ++entity.version;
            availableEntityIds.push(entity);
        }
    }
};

template <typename T>
class EntityManager : private EntityIDManager
{
    using Self            = EntityManager;
    using EntityContainer = Vector<T>;

    EntityContainer _entities;

public:
    struct RefIDPair
    {
        T&       entity;
        EntityID id;

        RefIDPair(T& entity, EntityID id) : entity{entity}, id{id} { }
    };

    EntityManager()
    {
        _entities.resize(maxEntities);
    }

    const T* get(const EntityID& id) const
    {
        if (id.getID() <= _entities.size())
        {
            auto index = id.getID();
            const T& e = _entities[index];
            if (e.id.valid() && e.id.getVersion() == id.getVersion())
            { return &e; }
        }
        return nullptr;
    }

    T* get(const EntityID& id)
    { return const_cast<T*>( const_cast<const Self*>(this)->get(id) ); }

    T& create(const T& copy = T(), EntityID specificID = EntityID())
    {
        EntityID id;
        if (specificID.null())
             { id = getFreeID(); }
        else { id = getSpecificID(specificID); }
        ASSERT(id.valid()); // ID is taken

        T& ref = _entities[id.getID()];
        ref    = copy;
        ref.id = id;

        return ref;
    }

    bool destroy(EntityID id)
    {
        if (T* e = get(id))
        {
            returnID(e->id);
            e->id.nullify();
            e->~T();
            new (e) T();
            return true;
        }
        return false;
    }

    void clear()
    {
        for (auto& e : _entities)
        { destroy(e.id); }
    }

    bool contains(const T& entity) const
    {
        const T* ptr = get(entity.id);
        return &entity == ptr;
    }

    const auto& container() const
    { return _entities; }

    size_t size() const
    { return entityCount; }

    bool empty() const
    { return entityCount == 0; }

    #pragma region Iterator Junk

    struct Iterator
    {
    private:
        T* ptr = nullptr;
        T* end = nullptr;

    public:
        Iterator(T* ptr, T* end)
        {
            this->ptr = ptr;
            this->end = end;
            while(this->ptr->id.null())
            { ++this->ptr; }
        }

        T& operator*() { return *ptr; }

        void operator++()
        {
            do { ++ptr; }
            while(ptr->id.null());
        }

        bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
    };

    struct ConstIterator
    {
    private:
        const T* ptr = nullptr;
        const T* end = nullptr;

    public:
        ConstIterator(const T* ptr, const T* end)
        {
            this->ptr = ptr;
            this->end = end;
            while(this->ptr->id.null())
            { ++this->ptr; }
        }

        const T& operator*() const { return *ptr; }

        void operator++()
        {
            do { ++ptr; }
            while(ptr->id.null());
        }

        bool operator!=(const ConstIterator& other) const { return ptr != other.ptr; }
    };

         Iterator begin()       { return      Iterator(_entities.begin(), _entities.end()); }
         Iterator end()         { return      Iterator(_entities.end(),   _entities.end()); }
    ConstIterator begin() const { return ConstIterator(_entities.begin(), _entities.end()); }
    ConstIterator end()   const { return ConstIterator(_entities.end(),   _entities.end()); }

    #pragma endregion
};
