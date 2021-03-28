#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Timer.hpp"

//-------------------------------------------------------------------------------------------------------------
class SpriteSheet;
class SpriteAnimDefinition;
//-------------------------------------------------------------------------------------------------------------



class Actor : public Entity 
{
public:
	Actor( EntityDef const& entityDef, Map* map );
	~Actor();

	virtual void Update( float deltaSeconds ) override;
	virtual void Render( Camera& camera ) override; 

	void SetAIState( AIState state );


public:
	Timer m_hurtTimer;

private:
	SpriteAnimDefinition*	m_walkAnimDef = nullptr;
	SpriteAnimDefinition*	m_idleAnimDef = nullptr;

	Timer	m_attackCoolDownTimer;
	float	m_attackTimer = 0.f;
	bool	m_hasAttackDamageApplied = false;
	float	m_turningSpeed = 360.f;
	float	m_flyingHeight = 0.f;

	float	m_deadTimerCount = 0.f;
};