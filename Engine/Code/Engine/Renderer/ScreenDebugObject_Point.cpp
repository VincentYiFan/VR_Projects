#include "ScreenDebugObject_Point.hpp"
#include "Engine/Renderer/DebugRender.hpp"

ScreenDebugObject_Point::ScreenDebugObject_Point( Vec2 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration )
{
	m_position = pos;
	m_size = size;
	
	m_startColor = start_color;
	m_endColor = end_color;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );


	m_box.SetCenter( pos );
	Vec2 center = m_box.GetCenter();
	m_box.mins = center - Vec2( 0.5f * size, 0.5f * size );
	m_box.maxs = center + Vec2( 0.5f * size, 0.5f * size );
}

ScreenDebugObject_Point::~ScreenDebugObject_Point()
{
}

void ScreenDebugObject_Point::DrawDebugObject()
{
	g_theDebugRenderSystem->m_context->DrawAABB2( m_box, m_startColor );
}
