#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Physics/Collider2D.hpp"

class DiscCollider2D : public Collider2D
{	
public:
	virtual void UpdateWorldShape() override;

	virtual AABB2 GetWorldBounds() const override;
	virtual Vec2 GetClosestPoint( Vec2 pos ) const override;
	virtual bool Contains( Vec2 pos ) const override;

	virtual void DebugRender( RenderContext* ctx, Rgba8 const& borderColor, Rgba8 const& fillColor ) override;

	virtual void Destroy() override;

	virtual float Calculatemoment( float mass ) override;

	~DiscCollider2D();

public:
	Vec2 m_localPosition; // my local offset from my parent
	Vec2 m_worldPosition; // calculated from local position and owning rigidbody if present
	float m_radius = 0.f;

	AABB2 m_worldBounds = AABB2::ZERO_TO_ONE;

	//Vec2 m_contactPoint = Vec2::ZERO;
};
