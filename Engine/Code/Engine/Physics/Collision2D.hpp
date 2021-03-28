#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <Vector>

class Collider2D;

struct Manifold2
{
public:
	Vec2	m_normal = Vec2::ZERO;	// The normal at the impact point
	float	m_peneration = 0.f;	    // How deeply inter-penetrated are the two objects.
	//Vec2	m_contactPoint = Vec2::ZERO;
	std::vector<Vec2> m_contactPoints;
};


struct Collision2D 
{
	Collider2D* me;
	Collider2D* them;
	Manifold2 manifold;

	Vec2		GetNormal() const { return manifold.m_normal; }
	float		GetPeneration() const { return manifold.m_peneration; }
	Collision2D GetInverse() const;
	IntVec2		GetID() const { return m_collisionID; }

	Vec2 GetContactPoint( Collider2D* collider );

	IntVec2 m_collisionID = IntVec2::ZERO;
};
