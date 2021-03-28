#include "Engine/Renderer/TextDebugObject.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"


TextDebugObject::TextDebugObject( Mat44 basis, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, char const* text )
{
	m_basis = basis;
	m_pivot = pivot;

	m_startColor = start_color;
	m_endColor = end_color;

	m_text = text;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );

	theFont = g_theDebugRenderSystem->m_context->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont");
}

TextDebugObject::TextDebugObject( Mat44 basis, Vec2 pivot, Rgba8 color, float fontSize, char const* text, float duration )
	:TextDebugObject( basis, pivot, color, color, duration, text )
{
	m_fontSize = fontSize;
}

TextDebugObject::~TextDebugObject()
{
}

void TextDebugObject::DrawDebugObject()
{
	std::vector<Vertex_PCU> textVerts;
	Vec2 dimensions = theFont->GetDimensionsForText2D( m_fontSize, m_text );
	Vec2 textMins = -( m_pivot * dimensions ); 
	theFont->AddVertsForText3D( textVerts, textMins, 0.f, m_fontSize, m_text, m_startColor );
	g_theDebugRenderSystem->m_context->BindTexture( theFont->GetTexture() );

	g_theDebugRenderSystem->m_context->SetModelMatrix( m_basis );
	g_theDebugRenderSystem->m_context->DrawVertexArray( textVerts );
}
