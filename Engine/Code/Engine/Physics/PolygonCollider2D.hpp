#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Edge.hpp"
#include "Engine/Math/LineSegment.hpp"
#include <vector>

class PolygonCollider2D : public Collider2D
{
public:
	virtual void UpdateWorldShape() override;
	void		 UpdateVertexes();

	virtual AABB2 GetWorldBounds() const override;

	bool IsValid() const; // must have at least 3 points to be considered a polygon

	bool IsConvex() const;
	Vec2 GetClosestPoint( Vec2 point ) const;
	float GetDistance( Vec2 point ) const;


	virtual bool Contains( Vec2 point ) const override;
	
	virtual float Calculatemoment( float mass ) override;

	// accessors
	int GetVertexCount() const;
	int GetEdgeCount() const;
	void GetEdge( int index, Vec2* outStart, Vec2* outEnd );
	Vec2 GetCenter() const;


	virtual void DebugRender( RenderContext* ctx, Rgba8 const& borderColor, Rgba8 const& fillColor ) override;

	virtual void Destroy() override;

	// GJK Intersection
	Vec2 Support( Vec2 dir ) const;
	Edge BestEdge( Vec2 dir ) const;

public:
	PolygonCollider2D( std::vector<Vec2> points );
	~PolygonCollider2D();

	//// create a convex wrapping of a collection of points;  
	//PolygonCollider2D MakeConvexFromPointCloud( Vec2 const* points, uint pointCount );

public:
	std::vector<Vec2> m_points;
	Vec2 m_localPosition; // my local offset from my parent
	Vec2 m_worldPosition; // calculated from local position and owning rigidbody if present
	Vec2 m_lastFrameWorldPosition;

	bool isPosInitialized = false;
	AABB2 m_worldBounds = AABB2::ZERO_TO_ONE;

private:
	float m_lastFrameRotationInRadians = 0.f;
};

