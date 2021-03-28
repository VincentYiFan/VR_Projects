#include "Engine/Renderer/MeshDebugObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"

MeshDebugObject::MeshDebugObject( Mat44 model, GPUMesh* mesh, Rgba8 start_tint, Rgba8 end_tint, float duration )
{
	m_model = model;
	m_mesh = mesh;

	m_startTint = start_tint;
	m_endTint = end_tint;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );
}

MeshDebugObject::~MeshDebugObject()
{
}

void MeshDebugObject::DrawDebugObject()
{
	g_theDebugRenderSystem->m_context->UpdateTintColor( m_startTint );
	g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_WIREFRAME );
	g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );

	g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_SOLID );
}
