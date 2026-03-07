#pragma once
#include "System.h"

#include <DirectXMath.h>
#include <vector>
#include <unordered_map>

class RigidBodyComponent;
class ColliderComponent;
class TransformComponent;
class Entity;

struct CollisionManifold
{
	Entity* entityA;
	Entity* entityB;
	DirectX::XMFLOAT3 normal;
	float depth;
	bool hasCollision;
};

class PhysicSystem : public System
{
	virtual void AddComponent(Component* _comp) override;
	virtual void Update() override;

	std::vector<ColliderComponent*> m_colliders;
	std::vector<RigidBodyComponent*> m_rigidBodies;

	std::vector<std::pair<int, int>> m_potentialCollisions;

private:

	float m_timerFixedUpdate = 0.f;

	void UpdatePhysic(RigidBodyComponent* _rigidBody, float dt);
	void UpdateAABB(ColliderComponent* collider, TransformComponent* transform);

	void BroadPhase();
	void NarrowPhase();

	CollisionManifold CheckSphereSphere(ColliderComponent* A, TransformComponent* tA, ColliderComponent* B, TransformComponent* tB);
	CollisionManifold CheckBoxBox(ColliderComponent* A, TransformComponent* tA, ColliderComponent* B, TransformComponent* tB);
	CollisionManifold CheckSphereBox(ColliderComponent* A, TransformComponent* tA, ColliderComponent* B, TransformComponent* tB);

	void ResolveCollision(const CollisionManifold& manifold);

	// Spatial Hashing pour le partitionning ---------------------------------------------

	const float m_cellSize = 10.0f;
	std::unordered_map<uint64_t, std::vector<int>> m_grid;

	uint64_t ComputeHash(int x, int y, int z) const {
		const uint64_t p1 = 73856093;
		const uint64_t p2 = 19349663;
		const uint64_t p3 = 83492791;
		return (static_cast<uint64_t>(x) * p1) ^
			(static_cast<uint64_t>(y) * p2) ^
			(static_cast<uint64_t>(z) * p3);
	}
};