#include "Engine/Renderer/BasisDebugObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"

BasisDebugObject::BasisDebugObject( Mat44 basis, Rgba8 start_tint, Rgba8 end_tint, float thickness, float duration )
{
	m_basis = basis;
	m_startTint = start_tint;
	m_endTint = end_tint;

	m_thickness = thickness;

	m_timer = new Timer();
	m_timer->SetSeconds( g_theDebugRenderSystem->m_clock, duration );

	m_mesh = new GPUMesh( g_theDebugRenderSystem->m_context );
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	Vec3 startPosition = basis.GetTranslation3D();

	AppendArrowToVerts( vertices, indices, startPosition, startPosition + basis.GetIBasis3D(), Rgba8::RED, Rgba8::RED, thickness );
	AppendArrowToVerts( vertices, indices, startPosition, startPosition + basis.GetJBasis3D(), Rgba8::GREEN, Rgba8::GREEN, thickness );
	AppendArrowToVerts( vertices, indices, startPosition, startPosition + basis.GetKBasis3D(), Rgba8::BLUE, Rgba8::BLUE, thickness );

	m_mesh->UpdateVertices( vertices );
	m_mesh->UpdateIndices( indices );
}

BasisDebugObject::~BasisDebugObject()
{
}

void BasisDebugObject::DrawDebugObject()
{
	g_theDebugRenderSystem->m_context->EnableDepth( eCompareOp::COMPARE_FUNC_ALWAYS, false );
	g_theDebugRenderSystem->m_context->DrawMesh( m_mesh );

	// Set Depth mode to default
	g_theDebugRenderSystem->m_context->EnableDepth( eCompareOp::COMPARE_FUNC_LEQUAL, false );
}
