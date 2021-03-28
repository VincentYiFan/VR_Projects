#include "BillboardTextDebugObject.hpp"

BillboardTextDebugObject::BillboardTextDebugObject( Vec3 origin, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, char const* text )
{
	UNUSED( pivot );

	m_origin = origin;

	m_startColor = start_color;
	m_endColor = end_color;

	m_text = text;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );

	theFont = g_theDebugRenderSystem->m_context->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont");

	m_mesh = new GPUMesh( g_theDebugRenderSystem->m_context );
}

BillboardTextDebugObject::~BillboardTextDebugObject()
{
}

void BillboardTextDebugObject::DrawDebugObject()
{
	std::vector<Vertex_PCU> textVerts;
	theFont->AddVertsForText3D( textVerts, Vec2( 0.f, 0.f ), 0.f, 0.5f, m_text, m_startColor );
	g_theDebugRenderSystem->m_context->BindTexture( theFont->GetTexture() );

	Mat44 cameraModel = g_theDebugRenderSystem->m_camera.GetViewMatrix();
	Mat44 lookAtMatrix = Mat44::CreateLookAtMatrix( m_origin, g_theDebugRenderSystem->m_camera.m_transform.m_position );
	lookAtMatrix.Translate3D( m_origin );

	g_theDebugRenderSystem->m_context->SetModelMatrix( lookAtMatrix );
	g_theDebugRenderSystem->m_context->DrawVertexArray( textVerts );
}
