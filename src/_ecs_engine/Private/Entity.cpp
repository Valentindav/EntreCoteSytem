#include "Public/Entity.h"

#include <Data/TransformData.h>

void Entity::AddChild(Entity* child)
{
    if (child->GetParent() == this) return;
    children.push_back(child);
    child->parent = this;
    child->transform.NewParent();
}

void Entity::RemoveChild(Entity* child)
{
    if (child->GetParent() != this) return;
    for (int i = 0; i < children.size(); i++)
        if (children[i] == child)
            children.erase(children.begin() + i);
    child->transform.RemoveParent();
    child->parent = nullptr;
}
