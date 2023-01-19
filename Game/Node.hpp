#pragma once

#include "Engine.hpp"
#include <vector>

using std::vector;

struct Node
{
    vector<SharedPtr<Node>> children;
    Node* parent;
    bool freed = false;

    auto& getChildren()
    { return children; }

    // TODO: emplace child

    void addChild(const SharedPtr<Node>& node)
    {
        ASSERTMSG(!node->parent, "Tried adding a node who already has a parent");
        node->parent = this;
        children.push_back(node);
    }

    void addChild(const Node& node)
    { addChild(make_shared<Node>(node)); }

    bool removeChild(const SharedPtr<Node>& child)
    {
        // https://stackoverflow.com/questions/14737780/how-do-i-remove-the-first-occurrence-of-a-value-from-a-vector
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end())
        {
            it->get()->parent = nullptr;
            children.erase(it);
            return true;
        }
        return false;
    }

    void clearChildren()
    {
        for (auto& c : children)
        { c->clearChildren(); c->parent = nullptr; }
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
    static SharedPtr<T> cast(SharedPtr<Node>& object)
    { return dynamic_cast<SharedPtr<T>>(object); }

    template <typename T>
    static SharedPtr<T> copy()
    { return make_shared<T>(*this); }

    virtual void input(const InputEvent& ev)
    {
        if (freed) { return; }
        for (auto& child : children)
        { child->input(ev); }
    }

    virtual void update(float delta)
    {
        std::erase_if(children, [](const auto& v) { return v->freed; });
        for (auto& child : children)
        { child->update(delta); }
    }

    virtual void draw(float delta)
    {
        for (auto& child : children)
        { child->draw(delta); }
    }

    virtual ~Node() { clearChildren(); };
};
