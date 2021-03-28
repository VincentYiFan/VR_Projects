#pragma once
#include "Game/Entity.hpp"

class Portal : public Entity
{
public:
	Portal( EntityDef const& entityDef, Map* map );
	~Portal();

	virtual void Update( float deltaSeconds ) override;
	virtual void Render( Camera& camera ) override; 

public:
	std::string		m_destMapStr;
	Vec2			m_destPos = Vec2::ZERO;
	float			m_destYawOffset = 0.f;
};
