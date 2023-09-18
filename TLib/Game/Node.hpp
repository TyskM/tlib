#pragma once

#include <TLib/Containers/Vector.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Media/Platform/Window.hpp>

struct Node
{
    Vector<Node> children;
    Node* parent;
    bool freed = false;

    auto& getChildren()
    { return children; }

    // TODO: emplace child

    void addChild(Node& node)
    {
        ASSERTMSG(!node.parent, "Tried adding a node who already has a parent");
        node.parent = this;
        children.push_back(node);
    }

    bool removeChild(const Node& child)
    {
        // https://stackoverflow.com/questions/14737780/how-do-i-remove-the-first-occurrence-of-a-value-from-a-vector
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end())
        {
            it->parent = nullptr;
            children.erase(it);
            return true;
        }
        return false;
    }

    void clearChildren()
    {
        for (auto& c : children)
        { c->clearChildren(); c.parent = nullptr; }
        children.clear();
    }

    Node* getParent() const
    { return parent; }

    inline void queueFree()
    { freed = true; }

    inline void unqueueFree()
    { freed = false; }

    template <class T>
    static T* cast(Node* object)
    { return dynamic_cast<T*>(object); }

    template <class T>
    static T* cast(Node& object)
    { return dynamic_cast<T*>(&object); }

    virtual void input(const InputEvent& ev)
    {
        if (freed) { return; }
        for (auto& child : children)
        { child.input(ev); }
    }

    virtual void update(float delta)
    {
        eastl::erase_if(children, [](const auto& v) { return v.freed; });
        for (auto& child : children)
        { child.update(delta); }
    }

    virtual void draw(float delta)
    {
        for (auto& child : children)
        { child.draw(delta); }
    }

    virtual ~Node() { clearChildren(); };
};
