#include "Engine/Renderer/ArrowDebugObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"

ArrowDebugObject::ArrowDebugObject( Vec3 p0, Rgba8 p0_start_color, Vec3 p1, Rgba8 p1_start_color, float thickness, float duration )
{
	m_start = p0;
	m_end = p1;

	m_p0_start_color = p0_start_color;
	m_p1_start_color = p1_start_color;

	m_thickness = thickness;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );


	m_mesh = new GPUMesh( g_theDebugRenderSystem->m_context );
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AppendArrowToVerts( vertices, indices, m_start, m_end, p0_start_color, p1_start_color, thickness );

	m_mesh->UpdateVertices( vertices );
	m_mesh->UpdateIndices( indices );
}

ArrowDebugObject::~ArrowDebugObject()
{
	delete m_timer;
	m_timer = nullptr;
}

void ArrowDebugObject::DrawDebugObject()
{
	g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );
}
