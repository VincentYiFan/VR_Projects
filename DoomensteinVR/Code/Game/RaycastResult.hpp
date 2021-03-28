#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Game/Entity.hpp"


//-------------------------------------------------------------------------------------------------------------
struct RaycastResult
{
public:
	Vec2	m_startPosition;
	Vec2	m_forwardNormal;
	float	m_maxDistance;
	bool	m_didImpact = false;
	Vec2	m_impactPosition;
	IntVec2 m_impactTileCoords;
	Entity*	m_impactEntity = nullptr;
	float	m_impactFraction;
	float	m_impactDistance;
	Vec2	m_impactSurfaceNormal;
};
