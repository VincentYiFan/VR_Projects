#include "ScreenDebugObject_Line.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Vec2.hpp"

ScreenDebugObject_Line::ScreenDebugObject_Line( Vec2 p0, Rgba8 p0_color, Vec2 p1, Rgba8 p1_color, float duration )
{
	m_p0 = p0;
	m_p1 = p1;

	m_p0_color = p0_color;
	m_p1_color = p1_color;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );
}

ScreenDebugObject_Line::~ScreenDebugObject_Line()
{
}

void ScreenDebugObject_Line::DrawDebugObject()
{
	Vec2 center = m_p0 + m_p1;
	Vec2 dir = m_p1 - m_p0;
	Vec2 iBasis = dir.GetRotatedMinus90Degrees().GetNormalized();
	OBB2 obb2 = OBB2( center, Vec2( 20.f, 100.f ), iBasis );  
	//g_theDebugRenderSystem->m_context->DrawOBB2( obb2, m_p0_color );

	g_theDebugRenderSystem->m_context->DrawLine( m_p0, m_p1, m_p0_color, 5.f );
}
