#include "Engine/Renderer/QuadDebugObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"


QuadDebugObject::QuadDebugObject(  Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3,AABB2 uvs, Rgba8 startColor, Rgba8 endColor, float duration )
{
	m_p0 = p0;
	m_p1 = p1;
	m_p2 = p2;
	m_p3 = p3;

	m_uvs = uvs;

	m_startColor = startColor;
	m_endColor = endColor;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );


	// Create GpuMesh
	m_mesh = new GPUMesh( g_theDebugRenderSystem->m_context );
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AppendQuadToVerts( vertices, indices, p0, p1, p2, p3, startColor );
	m_mesh->UpdateVertices( vertices );
	m_mesh->UpdateIndices( indices );
}


QuadDebugObject::~QuadDebugObject()
{
}

void QuadDebugObject::DrawDebugObject()
{
	g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );
}
