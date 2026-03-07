#pragma once

class Entity;

class NativeScript
{
public:
    Entity* owner = nullptr;

    virtual ~NativeScript() = default;

    virtual void OnStart() {}
    virtual void OnUpdate() {}
    virtual void OnDestroy() {}
    virtual void OnCollisionEnter() {}
    virtual void OnCollision() {}
    virtual void OnCollisionExit() {}

    virtual void OnCollisionEnter(Entity* other) { OnCollisionEnter(); }
    virtual void OnCollision(Entity* other) { OnCollision(); }
    virtual void OnCollisionExit(Entity* other) { OnCollisionExit(); }
};