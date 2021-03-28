#include "WireBoundDebugObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"

WireBoundDebugObject::WireBoundDebugObject( OBB3 obb3, Rgba8 startColor, Rgba8 endColor, float duration, bool isWireMode )
{
	m_startColor = startColor;
	m_endColor = endColor;

	m_isWireMode = isWireMode;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );

	m_mesh = new GPUMesh( g_theDebugRenderSystem->m_context );
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AppendOBB3ToVerts( vertices, indices, obb3, startColor );
	
	m_mesh->UpdateVertices( vertices );
	m_mesh->UpdateIndices( indices );
}

WireBoundDebugObject::WireBoundDebugObject( AABB3 aabb3, Rgba8 startColor, Rgba8 endColor, float duration, bool isWireMode )
{
	m_startColor = startColor;
	m_endColor = endColor;

	m_isWireMode = isWireMode;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );

	m_mesh = new GPUMesh( g_theDebugRenderSystem->m_context );
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AppendIndexedCubeToVerts( vertices, indices, aabb3, startColor );

	m_mesh->UpdateVertices( vertices );
	m_mesh->UpdateIndices( indices );
}

WireBoundDebugObject::~WireBoundDebugObject()
{
}

void WireBoundDebugObject::DrawDebugObject()
{
	if( m_isWireMode )
	{
		//g_theDebugRenderSystem->m_context->UpdateTintColor( m_startColor );
		g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_WIREFRAME );
		g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );

		g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_SOLID );

	}
	else {
		//g_theDebugRenderSystem->m_context->UpdateTintColor( m_startColor );
		g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_SOLID );
		g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );
	}

	//g_theDebugRenderSystem->m_context->UpdateTintColor( Rgba8::WHITE );
}
