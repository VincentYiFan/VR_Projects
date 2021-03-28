#include "Engine/Renderer/LineDebugObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"

LineDebugObject::LineDebugObject( Vec3 p0, Rgba8 p0_start_color, Vec3 p1, Rgba8 p1_start_color,float thickness, float duration, bool isWire )
{
	m_p0_start_color = p0_start_color;
	m_p1_start_color = p1_start_color;

	m_thickness = thickness;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );
	m_isWire = isWire;

	m_mesh = new GPUMesh( g_theDebugRenderSystem->m_context );
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AppendLineToVerts( vertices, indices, p0, p1, p0_start_color, p1_start_color, thickness );
	m_mesh->UpdateVertices( vertices );
	m_mesh->UpdateIndices( indices );
}

LineDebugObject::~LineDebugObject()
{
}

void LineDebugObject::DrawDebugObject()
{
	if( !m_isWire )
	{
		g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );
	}
	else
	{
		g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_WIREFRAME );
		g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );

		g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_SOLID );
	}
}
