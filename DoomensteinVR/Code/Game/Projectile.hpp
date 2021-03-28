#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/Transform.hpp"

class GPUMesh;

class Projectile : public Entity
{
public:
	Projectile( EntityDef const& entityDef, Map* map );
	//Projectile( EntityDef const& entityDef, Map* map, Faction faction, const Vec2& position );
	~Projectile();

	virtual void	Update( float deltaSeconds ) override;
	virtual void	Render( Camera& camera ) override;
	virtual void	DebugRender( Camera& camera ) const override;	// #ToDo: const might be unnecessary
	void			CheckCollision();

	void			SetVelocity( Vec3 velocity );
	
public:
	Transform	m_transform;
	GPUMesh*	m_mesh = nullptr;
	Vec3		m_velocity = Vec3::ZERO;
	bool		m_hasAppliedDamage = false;
};
