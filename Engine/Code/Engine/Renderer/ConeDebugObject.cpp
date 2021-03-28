#include "Engine/Renderer/ConeDebugObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/math/Cone.hpp"

ConeDebugObject::ConeDebugObject( const Cone& cone, Rgba8 startColor, Rgba8 endColor, float duration )
{
	m_transform.SetPosition( cone.m_center );
	m_startColor = startColor;
	m_endColor = endColor;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );

	m_mesh = new GPUMesh( g_theDebugRenderSystem->m_context );
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	// #ToDo: Push the vertices into the vector 
	AddConeToVerts( vertices, indices, cone.m_center, cone.m_radius, cone.m_apexPoint, cone.m_height, startColor, endColor );
	m_mesh->UpdateVertices( vertices );
	m_mesh->UpdateIndices( indices );
}

ConeDebugObject::~ConeDebugObject()
{
}

void ConeDebugObject::DrawDebugObject()
{
	g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );
}
