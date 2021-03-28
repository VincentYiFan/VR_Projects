#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"

void DiscCollider2D::UpdateWorldShape()
{
	if( m_rigidbody != nullptr ) {
		m_worldPosition = m_rigidbody->m_worldPosition;
	}
	else {
		m_worldPosition = m_localPosition;
	}

	m_worldBounds = GetWorldBounds();
}

AABB2 DiscCollider2D::GetWorldBounds() const
{
	AABB2 discBoundingBox = AABB2( m_worldPosition - Vec2( m_radius, m_radius ), m_worldPosition + Vec2( m_radius, m_radius ) );
	return discBoundingBox;
}

Vec2 DiscCollider2D::GetClosestPoint( Vec2 pos ) const
{
	return GetNearestPointOnDisc2D( pos, m_worldPosition, m_radius );
}

bool DiscCollider2D::Contains( Vec2 pos ) const
{
	return( IsPointInsideDisc2D( pos, m_worldPosition, m_radius ) );
}

void DiscCollider2D::DebugRender( RenderContext* ctx, Rgba8 const& borderColor, Rgba8 const& fillColor )
{
	ctx->BindTexture( nullptr );

	ctx->DrawDisc( m_worldPosition, m_radius, fillColor );
	ctx->DrawRing( m_worldPosition, m_radius, borderColor, 0.5f );

	float rotationRadians = m_rigidbody->GetRotationInRadians();

	// Render the rigidbody's world position
	Vec2 topRight    = m_worldPosition + Vec2( .5f, .5f ).GetRotatedRadians( rotationRadians );
	Vec2 topLeft	 = m_worldPosition + Vec2( -.5f, .5f ).GetRotatedRadians( rotationRadians );
	Vec2 bottomLeft  = m_worldPosition + Vec2( -.5f, -.5f ).GetRotatedRadians( rotationRadians );
	Vec2 bottomRight = m_worldPosition + Vec2( .5f, -.5f ).GetRotatedRadians( rotationRadians );

	Vec2 zeroDegreesPointOnDisc = m_worldPosition + ( m_radius * Vec2( cosf( rotationRadians ), sinf( rotationRadians ) ) );
	ctx->DrawLine( m_worldPosition, zeroDegreesPointOnDisc, Rgba8::GREY, 0.3f );

	Rgba8 rigidbodyPosColor = Rgba8::BLUE;
	if( !m_rigidbody->m_isEnabled ) {
		rigidbodyPosColor = Rgba8::RED;
	}
	ctx->DrawLine( bottomLeft, topRight, rigidbodyPosColor, 0.3f );
	ctx->DrawLine( bottomRight, topLeft, rigidbodyPosColor, 0.3f );


	// Render contact point
	//ctx->DrawLine( m_contactPoint + Vec2( -.5f, -.5f ),  m_contactPoint + Vec2( .5f, .5f ), Rgba8::MAGENTA, 0.3f );
	//ctx->DrawLine(  m_contactPoint + Vec2( .5f, -.5f ), m_contactPoint + Vec2( -.5f, .5f ), Rgba8::MAGENTA, 0.3f );

	// Render the world bounds
	//ctx->DrawAABB2( m_worldBounds, Rgba8( 150, 150, 150, 100 ) );
}

void DiscCollider2D::Destroy()
{
	m_isGarbage = true;
}

float DiscCollider2D::Calculatemoment( float mass )
{
	// 0.5f * m * r^2
	float moment = 0.5f * mass * ( m_radius * m_radius );
	return moment;
}

DiscCollider2D::~DiscCollider2D()
{
}
