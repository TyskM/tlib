#pragma once

#include <TLib/Game/Node.hpp>
#include <TLib/Game/Transform2D.hpp>

struct Node2D : Node
{
private:
    Transform2D localTransform;
    Transform2D globalTransform;
    bool globalTransformDirty = true;

public:

    void setPos(const Vector2f& pos)
    {
        localTransform.pos = pos;
        onTransformChanged();
    }

    Vector2f getPos()
    { return localTransform.pos; }

    void setScale(const Vector2f& scale)
    {
        localTransform.scale = scale;
        onTransformChanged();
    }

    Vector2f getScale() { return localTransform.scale; }

    void setRot(const float rot)
    {
        localTransform.rot = rot;
        onTransformChanged();
    }

    float getRot() { return localTransform.rot; }

    void setTransform(const Transform2D& t)
    {
        localTransform = t;
        onTransformChanged();
    }

    Transform2D getTransform()
    { return localTransform; }

    Transform2D getGlobalTransform()
    {
        if (globalTransformDirty)
        {
            Node2D* parent2d = cast<Node2D>(getParent());
            if (parent2d)
            { globalTransform = parent2d->getGlobalTransform() + getTransform(); }
            else
            { globalTransform = getTransform(); }

            globalTransformDirty = false;
        }
        return globalTransform;
    }

    void onTransformChanged()
    {
        globalTransformDirty = true;
        for (auto& child : children)
        {
            Node2D* child2d = Node::cast<Node2D>(child);
            if (child2d) { child2d->globalTransformDirty = true; }
        }
    }

    virtual ~Node2D() = default;
};
