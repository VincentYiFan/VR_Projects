#include "Engine/Renderer/SphereDebugObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"

SphereDebugObject::SphereDebugObject( Vec3 pos, float radius, Rgba8 startColor, Rgba8 endColor, float duration, bool isWireMode,  eDebugRenderMode mode )
{
	m_transform.SetPosition( pos ); // Fix
	m_radius = radius;
	m_startColor = startColor;
	m_endColor = endColor;

	// Setup Debug Render Mode
	m_mode = mode;

	m_isWireMode = isWireMode;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );

	m_mesh = new GPUMesh( g_theDebugRenderSystem->m_context );
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AddUVSphereToIndexedVertexArray( vertices, indices, pos, radius, 10, 10, Rgba8::WHITE );
	m_mesh->UpdateVertices( vertices );
	m_mesh->UpdateIndices( indices );
}

SphereDebugObject::~SphereDebugObject()
{
}

void SphereDebugObject::DrawDebugObject()
{
	if( m_isWireMode )
	{
		g_theDebugRenderSystem->m_context->UpdateTintColor( m_startColor );
		g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_WIREFRAME );
		g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );

		g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_SOLID );
	}
	else {
		g_theDebugRenderSystem->m_context->UpdateTintColor( m_startColor );
		g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );
	}

	if( m_mode == DEBUG_RENDER_XRAY )
	{
		g_theDebugRenderSystem->m_context->EnableDepth( eCompareOp::COMPARE_FUNC_GREATER, false );
		Rgba8 xrayColor = Rgba8( (char)(m_startColor.r * 0.5f), (char)(m_startColor.g * 0.5f), (char)(m_startColor.b * 0.5f), m_startColor.a );
		g_theDebugRenderSystem->m_context->UpdateTintColor( xrayColor );

		if( m_isWireMode )
		{
			g_theDebugRenderSystem->m_context->UpdateTintColor( m_startColor );
			g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_WIREFRAME );
			g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );

			g_theDebugRenderSystem->m_context->SetFillMode( D3D11_FILL_SOLID );
		}
		else {
			g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );
		}
	}
	g_theDebugRenderSystem->m_context->UpdateTintColor( Rgba8::WHITE );
}
