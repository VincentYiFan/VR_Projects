#include "Engine/Renderer/GPUMesh.hpp"

GPUMesh::GPUMesh( RenderContext* owner )
	:m_owner( owner )
{
	m_vertices = new VertexBuffer( m_owner, MEMORY_HINT_DYNAMIC );
	m_indices = new IndexBuffer( m_owner, MEMORY_HINT_DYNAMIC );
}

GPUMesh::~GPUMesh()
{
	delete m_vertices;
	m_vertices = nullptr;

	delete m_indices;
	m_indices = nullptr;

	m_owner = nullptr;
}

void GPUMesh::UpdateVertices( uint vcount, void const* vertexData, uint vertexStride, buffer_attribute_t const* layout )
{
	m_verticesCount = vcount;

	m_vertices->Update( vcount, vertexData, vertexStride, layout );
}

void GPUMesh::UpdateVertices( std::vector<Vertex_PCU> const& vertices )
{
	UpdateVertices( (uint)vertices.size(),
		&vertices[0],
		sizeof( Vertex_PCU ),
		Vertex_PCU::LAYOUT );
}

void GPUMesh::UpdateVertices( std::vector<Vertex_PCUTBN> const& vertices )
{
	UpdateVertices( (uint)vertices.size(),
		&vertices[0],
		sizeof( Vertex_PCUTBN ),
		Vertex_PCUTBN::LAYOUT );
}


void GPUMesh::UpdateIndices( uint icount, uint const* indices )
{
	m_indicesCount = icount;

	if( indices != nullptr )
	{
		m_indices->Update( icount, indices);
	}
}

void GPUMesh::UpdateIndices( std::vector<uint> const& indices )
{
	UpdateIndices( (uint)indices.size(), &indices[0] );
}

int GPUMesh::GetIndexCount() const
{
	return m_indicesCount;
}

int GPUMesh::GetVertexCount() const
{
	return m_verticesCount;
}
