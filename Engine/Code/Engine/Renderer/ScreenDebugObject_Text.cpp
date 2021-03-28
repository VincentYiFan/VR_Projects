#include "Engine/Renderer/ScreenDebugObject_Text.hpp"
#include "Engine/Renderer/DebugRender.hpp"


ScreenDebugObject_Text::ScreenDebugObject_Text( Vec4 AlignmentOffest, Vec2 pivot, float textSize, Rgba8 start_color, Rgba8 end_color, float duration, char const* text )
{
	m_alignmentOffset = AlignmentOffest;
	m_pivot = pivot;

	m_textSize = textSize;
	m_startColor = start_color;
	m_endColor = end_color;

	m_text =  text;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );

	m_theFont = g_theDebugRenderSystem->m_context->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont");
}

ScreenDebugObject_Text::~ScreenDebugObject_Text()
{
	m_theFont = nullptr;

	delete m_timer;
	m_timer = nullptr;
}

void ScreenDebugObject_Text::DrawDebugObject()
{
	std::vector<Vertex_PCU> textVerts;

	Vec2 alignment = Vec2( m_alignmentOffset.x, m_alignmentOffset.y );
	Vec2 offset = Vec2( m_alignmentOffset.z, m_alignmentOffset.w );

	Vec2 textMins = alignment * g_theDebugRenderSystem->m_screenCameraDimensions.maxs;
	m_theFont->AddVertsForText2D( textVerts, textMins, m_textSize, m_text, m_startColor, 1.f );

	g_theDebugRenderSystem->m_context->BindTexture( m_theFont->GetTexture() );
	g_theDebugRenderSystem->m_context->DrawVertexArray( textVerts );

	textVerts.clear();
}
