#include "Public/Entity.h"

#include <Data/TransformData.h>

void Entity::AddChild(Entity* child)
{
    if (child->parent != nullptr) {
        child->parent->RemoveChild(child);
    }
    child->parent = this;
    children.push_back(child);
    child->transform.NewParent();
}

void Entity::RemoveChild(Entity* child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
        child->parent = nullptr;
        child->transform.NewParent();
    }
}

void Entity::SetParent(Entity* parent) { parent->AddChild(this); }

void Entity::RemoveParent() { parent->RemoveChild(this); }

Entity* Entity::GetRoot()
{
    Entity* current = this;
    while (current->parent != nullptr)
    {
        current = current->parent;
    }
    return current;
}