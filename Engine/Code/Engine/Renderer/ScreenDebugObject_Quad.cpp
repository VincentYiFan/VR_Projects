#include "Engine/Renderer/ScreenDebugObject_Quad.hpp"
#include "Engine/Renderer/DebugRender.hpp"

ScreenDebugObject_Quad::ScreenDebugObject_Quad( AABB2 bounds, Rgba8 start_color, Rgba8 end_color, float duration )
{
	m_box = bounds;
	m_startColor = start_color;
	m_endColor = end_color;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );
}

ScreenDebugObject_Quad::ScreenDebugObject_Quad( AABB2 bounds, Rgba8 tint, Texture* tex, AABB2 uvs, float duration )
{
	m_box = bounds;
	m_tint = tint;

	m_tex = tex;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );

	m_uvs = uvs;
}

ScreenDebugObject_Quad::~ScreenDebugObject_Quad()
{
}

void ScreenDebugObject_Quad::DrawDebugObject()
{
	if( m_tex != nullptr ) 
	{
		g_theDebugRenderSystem->m_context->BindTexture( m_tex );
	}

	//g_theDebugRenderSystem->m_context->DrawQuad( m_box, 0.f, m_startColor );

	std::vector<Vertex_PCU> verts;
	verts.emplace_back( Vec2( m_box.mins.x, m_box.mins.y ), m_startColor, Vec2( m_uvs.mins.x, m_uvs.mins.y ) );
	verts.emplace_back( Vec2( m_box.maxs.x, m_box.mins.y ), m_startColor, Vec2( m_uvs.maxs.x, m_uvs.mins.y ) );
	verts.emplace_back( Vec2( m_box.maxs.x, m_box.maxs.y ), m_startColor, Vec2( m_uvs.maxs.x, m_uvs.maxs.y ) );

	verts.emplace_back( Vec2( m_box.mins.x, m_box.mins.y ), m_startColor, Vec2( m_uvs.mins.x, m_uvs.mins.y ) );
	verts.emplace_back( Vec2( m_box.maxs.x, m_box.maxs.y ), m_startColor, Vec2( m_uvs.maxs.x, m_uvs.maxs.y ) );
	verts.emplace_back( Vec2( m_box.mins.x, m_box.maxs.y ), m_startColor, Vec2( m_uvs.mins.x, m_uvs.maxs.y ) );

	g_theDebugRenderSystem->m_context->DrawVertexArray( verts );
}
