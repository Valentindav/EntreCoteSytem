#include "Component.h"

#include "Public/ECS.h"
#include "Public/Entity.h"
#include "Private/EngineCore.h"

Entity* Component::GetOwner()
{
	return ECS_ENGINE->GetECS()->GetEntity(mOwnerID);
}