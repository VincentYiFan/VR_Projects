#pragma once
#include "Engine/Core/Vertex_PCU.hpp"

struct ID3D11Buffer;
class RenderContext;

typedef unsigned int uint;
typedef unsigned char byte;
#define BIT_FLAG(b)		(1 << (b))

// BIT_FLAG is a custom macro, sets the bit 
enum eRenderBufferUsageBit : uint
{
	VERTEX_BUFFER_BIT 		= BIT_FLAG( 0 ),		// A02: can be used to store vertices
	INDEX_BUFFER_BIT		= BIT_FLAG( 1 ),  		// A05: Index Buffer (IBO)
	UNIFORM_BUFFER_BIT		= BIT_FLAG( 2 ),		// A03: used to store constants
};
typedef uint eRenderBufferUsage;

enum eRenderMemoryHint
{
	MEMORY_HINT_GPU,		// memory updated and used only from GPU side (if created from CPU, must be initialization data) // GPU can read/write, CPU can't touch it
	MEMORY_HINT_DYNAMIC, 	// something updated often from CPU side, used by GPU side // It allows to 'Map' the memory
	MEMORY_HINT_STAGING,  // memory that can be read/written from CPU, and copied to a GPU buffer e.g, screenshot
};

// A GPU Allocator
class RenderBuffer
{
public:
	RenderBuffer( RenderContext* owner, eRenderBufferUsage usage, eRenderMemoryHint memoryHint );
	~RenderBuffer();

	bool Update( void const* data, size_t dataByteSize, size_t elementByteSize );

private:
	bool Create( size_t dataByteSize, size_t elementByteSize );
	bool IsCompatible( size_t dataByteSize, size_t elementByteSize ) const;
	void CleanUp();

public:
	RenderContext* m_owner = nullptr;
	ID3D11Buffer* m_handle = nullptr;

	eRenderBufferUsage m_usage;
	eRenderMemoryHint m_memoryHint;

	size_t m_bufferByteSize;
	size_t m_elementByteSize;

};

class VertexBuffer : public RenderBuffer
{
public:
	VertexBuffer( RenderContext* ctx, eRenderMemoryHint hint );

	void Update( uint vcount, void const* vertexData, uint vertexStride, buffer_attribute_t const* layout );
	buffer_attribute_t const* GetLayout() { return m_layout; }
	const uint GetElementStride() const { return m_elementStride; }

public:
	buffer_attribute_t const* m_layout; // array describing the vertex
	uint m_elementStride;
};
