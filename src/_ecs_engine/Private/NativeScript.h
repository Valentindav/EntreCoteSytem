#pragma once

class Entity;

class NativeScript
{
protected:

    Entity* owner = nullptr;

    virtual ~NativeScript() = default;

    virtual void OnStart() {}
    virtual void OnUpdate() {}
    virtual void OnDestroy() {}

    virtual void OnCollisionEnter(Entity* other) {}
    virtual void OnCollision(Entity* other) {}
    virtual void OnCollisionExit(Entity* other) {}

    friend class ScriptComponent;
    friend class ScriptSystem;
};