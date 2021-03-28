#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/LineSegment.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Collision2D Collision2D::GetInverse() const
{
	Collision2D inverseCol;
	inverseCol.me = them;
	inverseCol.them = me;
	inverseCol.manifold = manifold;
	inverseCol.manifold.m_normal = -inverseCol.manifold.m_normal;
	inverseCol.m_collisionID = GetID();

	return inverseCol;
}

Vec2 Collision2D::GetContactPoint( Collider2D* collider )
{
	//for( int idx = 0; idx < (int)manifold.m_contactPoints.size(); ++idx ) {
	//	g_theDebugRenderSystem->DebugAddScreenPoint( manifold.m_contactPoints[idx], 1.f, Rgba8::MAGENTA, 0.1f );
	//}

	Vec2 contactPoint = Vec2::ZERO;
	DiscCollider2D* disc = dynamic_cast<DiscCollider2D*>( collider );
	PolygonCollider2D* polygon = dynamic_cast<PolygonCollider2D*>( collider );

	DiscCollider2D* meDiscCollider = dynamic_cast<DiscCollider2D*>( me );
	DiscCollider2D* themDiscCollider = dynamic_cast<DiscCollider2D*>( them );
	//PolygonCollider2D* mePolygonCollider = dynamic_cast<PolygonCollider2D*>( me );
	//PolygonCollider2D* themPolygonCollider = dynamic_cast<PolygonCollider2D*>( them );

	bool isColliderMe = false;
	bool isColliderThem = false;
	if( collider == me ) {
		isColliderMe=  true;
	} else {
		isColliderThem = true;
	}


	if( polygon != nullptr )
	{
		const int numContactPoints = (int)manifold.m_contactPoints.size();
		if( numContactPoints >= 2 )
		{
			Vec2 p0 = manifold.m_contactPoints[0];
			Vec2 p1 = manifold.m_contactPoints[1];
			LineSegment line = LineSegment( manifold.m_contactPoints[0], p1-p0, GetDistance2D( p0, p1 ) );

			if( isColliderMe ) {
				contactPoint = line.GetNearestPoint( me->m_rigidbody->m_worldPosition );
				return contactPoint;
			}
			if( isColliderThem ) {
				contactPoint = line.GetNearestPoint( them->m_rigidbody->m_worldPosition );
				return contactPoint;
			}
		} 
		else if( numContactPoints == 1 )
		{
			contactPoint = manifold.m_contactPoints[0];
			return contactPoint;
		}
		//else {
		//	ERROR_AND_DIE( "Wrong number of points" );
		//}
	}

	if( disc != nullptr )
	{
		if( isColliderMe )
		{
			contactPoint = meDiscCollider->m_worldPosition - meDiscCollider->m_radius * GetNormal();
			return contactPoint;
		}
		if( isColliderThem )
		{
			contactPoint = themDiscCollider->m_worldPosition + themDiscCollider->m_radius * GetNormal();
			return contactPoint;
		}
	}

	// ToDo: Fix the return place 
	return contactPoint;
}
