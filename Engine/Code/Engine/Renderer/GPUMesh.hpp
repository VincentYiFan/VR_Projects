#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"


class GPUMesh
{
public:
	GPUMesh( RenderContext* owner );
	~GPUMesh();

	void UpdateVertices( uint vcount, void const* vertexData, uint vertexStride, buffer_attribute_t const* layout );
	void UpdateVertices( std::vector<Vertex_PCU> const& vertices );
	void UpdateVertices( std::vector<Vertex_PCUTBN> const& vertices );

	void UpdateIndices( uint icount, uint const* indices );
	void UpdateIndices( std::vector<uint> const& indices );

	int GetIndexCount() const;
	int GetVertexCount() const;

	VertexBuffer* GetVertexBuffer() const { return m_vertices; }
	IndexBuffer*  GetIndexBuffer()	const { return m_indices; }

	// helper template
	template <typename VERTEX_TYPE>
	void UpdateVertices( uint vcount, VERTEX_TYPE const* vertices )
	{
		UpdateVertices( vcount, vertices, sizeof( VERTEX_TYPE ), VERTEX_TYPE::LAYOUT );
	}


public:
	VertexBuffer* m_vertices = nullptr;  // like a std::vector<VertexPCU> -> but on GPU
	IndexBuffer* m_indices = nullptr; 	// like a std::vector<uint> -> but on the GPU

	uint m_verticesCount = 0;
	uint m_indicesCount = 0;

	RenderContext* m_owner = nullptr;
};
