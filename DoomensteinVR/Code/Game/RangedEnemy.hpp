#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Timer.hpp"

class RangedEnemy : public Entity
{
public :
	RangedEnemy( EntityDef const& entityDef, Map* map );
	~RangedEnemy();

	virtual void Update( float deltaSeconds ) override;
	virtual void Render( Camera& camera ) override;

	void SpawnProjectile();
	Vec3 GetProjectileSpawnLocation() const; 

private:
	Timer	m_attackCoolDownTimer;
	float	m_deathTimerCount = 0.f;
};
