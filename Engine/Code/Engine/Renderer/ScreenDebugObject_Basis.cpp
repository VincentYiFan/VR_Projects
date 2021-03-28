#include "ScreenDebugObject_Basis.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"


ScreenDebugObject_Basis::ScreenDebugObject_Basis( Vec2 screen_origin_location, Mat44 basis_to_render, Rgba8 start_tint, Rgba8 end_tint, float duration )
{
	m_position = screen_origin_location;

	m_iBasis = basis_to_render.GetIBasis2D();
	m_jBasis = basis_to_render.GetJBasis2D();

	m_startTint = start_tint;
	m_endTint = end_tint;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );
}

ScreenDebugObject_Basis::~ScreenDebugObject_Basis()
{
}

void ScreenDebugObject_Basis::DrawDebugObject()
{
	// Draw arrow

	float boxWidth = 100.f;
	float boxHeight = 7.5f;
	AABB2 box = AABB2( m_position, m_position + Vec2( boxWidth, boxHeight ) );
	
	Vec2 pointToDrawTriangle = 0.5f * ( box.maxs + Vec2( box.maxs.x, box.mins.y ) );

	Vec2 p0 = pointToDrawTriangle + m_jBasis * boxHeight * 2.f;
	Vec2 p1 = pointToDrawTriangle - m_jBasis * boxHeight * 2.f;
	Vec2 p2 = pointToDrawTriangle + (m_iBasis * boxHeight * 1.717f  * 2.f);

	std::vector<Vertex_PCU> verts;
	verts.push_back( Vertex_PCU( p0, Rgba8::WHITE, Vec2::ZERO ) );
	verts.push_back( Vertex_PCU( p1, Rgba8::WHITE, Vec2::ZERO ) );
	verts.push_back( Vertex_PCU( p2, Rgba8::WHITE, Vec2::ZERO ) );

	AppendAABB2D( verts, box, Rgba8::WHITE );

	g_theDebugRenderSystem->m_context->UpdateTintColor( Rgba8::RED );

	//g_theDebugRenderSystem->m_context->DrawAABB2( box, Rgba8::RED );
	g_theDebugRenderSystem->m_context->DrawVertexArray( verts );

	AABB2 box_2 = AABB2( m_position, m_position + Vec2( boxHeight, boxWidth ) );
	Vec2 pointToDrawTriangle_2 = 0.5f * ( box_2.maxs + Vec2( box_2.mins.x, box_2.maxs.y ) );

	Vec2 p3 = pointToDrawTriangle_2 + m_iBasis * boxHeight * 2.f;
	Vec2 p4 = pointToDrawTriangle_2 - m_iBasis * boxHeight * 2.f;
	Vec2 p5 = pointToDrawTriangle_2 + (m_jBasis * boxHeight * 1.717f  * 2.f);

	std::vector<Vertex_PCU> verts_2;
	verts_2.push_back( Vertex_PCU( p3, Rgba8::WHITE, Vec2::ZERO ) );
	verts_2.push_back( Vertex_PCU( p4, Rgba8::WHITE, Vec2::ZERO ) );
	verts_2.push_back( Vertex_PCU( p5, Rgba8::WHITE, Vec2::ZERO ) );

	AppendAABB2D( verts_2, box_2, Rgba8::WHITE );


	g_theDebugRenderSystem->m_context->UpdateTintColor( Rgba8::GREEN );
	g_theDebugRenderSystem->m_context->DrawVertexArray( verts_2 );
}
