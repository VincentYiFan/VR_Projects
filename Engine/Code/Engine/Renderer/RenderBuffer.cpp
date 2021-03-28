#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

RenderBuffer::RenderBuffer( RenderContext* owner, eRenderBufferUsage usage, eRenderMemoryHint memoryHint )
{
	m_owner = owner;
	m_usage = usage;
	m_memoryHint = memoryHint;

	m_handle = nullptr;
	m_bufferByteSize = 0U;
	m_elementByteSize = 0U;
}

RenderBuffer::~RenderBuffer()
{
	DX_SAFE_RELEASE(m_handle);
}

UINT ToDXUsage( eRenderBufferUsage usage )
{
	UINT ret = 0;

	if( usage & VERTEX_BUFFER_BIT )
	{
		ret |= D3D11_BIND_VERTEX_BUFFER;
	}

	if( usage & INDEX_BUFFER_BIT )
	{
		ret |= D3D11_BIND_INDEX_BUFFER;
	}

	if( usage & UNIFORM_BUFFER_BIT )
	{
		ret |= D3D11_BIND_CONSTANT_BUFFER;
	}

	return ret;
}

D3D11_USAGE ToDXMemoryUsage( eRenderMemoryHint hint )
{
	switch( hint )
	{
	case MEMORY_HINT_GPU:		return D3D11_USAGE_DEFAULT;
	case MEMORY_HINT_DYNAMIC:	return D3D11_USAGE_DYNAMIC;
	case MEMORY_HINT_STAGING:	return D3D11_USAGE_STAGING;
	default:					return D3D11_USAGE_DEFAULT;
	}
}

bool RenderBuffer::Create( size_t dataByteSize, size_t elementByteSize )
{
	ID3D11Device* device = m_owner->m_device;

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (UINT)dataByteSize;
	desc.Usage = ToDXMemoryUsage( m_memoryHint );
	desc.BindFlags = ToDXUsage( m_usage );
	desc.CPUAccessFlags = 0;

	if( m_memoryHint == MEMORY_HINT_DYNAMIC )
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if( m_memoryHint == MEMORY_HINT_STAGING )
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	}

	desc.MiscFlags = 0;
	desc.StructureByteStride = (UINT)elementByteSize;
	device->CreateBuffer( &desc, nullptr, &m_handle );

	// testing
	//std::string name = "Test Name";
	//device->SetPrivateData( WKPDID_D3DDebugObjectName, name.size(), name.c_str() );

	m_bufferByteSize = dataByteSize;
	m_elementByteSize = elementByteSize;


	return( m_handle != nullptr );
}

bool RenderBuffer::IsCompatible( size_t dataByteSize, size_t elementByteSize ) const
{
	if( m_handle == nullptr )
	{
		return false;
	}

	if( m_elementByteSize != elementByteSize )
	{
		return false;
	}

	if( m_memoryHint == MEMORY_HINT_DYNAMIC )
	{
		return dataByteSize <= m_bufferByteSize;
	}

	else
	{
		return dataByteSize == m_bufferByteSize;
	}
}

void RenderBuffer::CleanUp()
{
	DX_SAFE_RELEASE( m_handle );
	m_bufferByteSize = 0;
	m_elementByteSize = 0;
}

bool RenderBuffer::Update( void const* data, size_t dataByteSize, size_t elementByteSize )
{
	// 1. if not compatible - destroy the old buffer
	if( !IsCompatible( dataByteSize, elementByteSize ) )
	{
		CleanUp();  // destroy the buffer

		// 2. if no buffer, create one that is compatible
		Create( dataByteSize, elementByteSize );
	}
	
	

	ID3D11DeviceContext* ctx = m_owner->m_context;
	// 3. updating the buffer
	if( m_memoryHint == MEMORY_HINT_DYNAMIC )
	{
		D3D11_MAPPED_SUBRESOURCE mapped;

		// CPU -> GPU memory copy
		HRESULT result = ctx->Map( m_handle, 0, D3D11_MAP_WRITE_DISCARD, 0,  &mapped );
		if( SUCCEEDED( result ) )
		{
			memcpy( mapped.pData, data, dataByteSize );
			ctx->Unmap( m_handle, 0 );
		}
		else
		{
			return false;
		}
		
	}
	else
	{ // if this is MEMORY_HINT_GPU
		ctx->UpdateSubresource( m_handle, 0, nullptr, data, 0, 0 );
	}

	// Mapping
	// Only available to DYNAMIC buffer
	// But, don't gave to reallocate if going smaller

	// CopySubresource ( direct copy)
	// This is only available to GPU buffers that have exactly
	// the same size, and element size
	return true;
}

VertexBuffer::VertexBuffer( RenderContext* ctx, eRenderMemoryHint hint )
	:RenderBuffer( ctx, VERTEX_BUFFER_BIT, hint )
{
}

void VertexBuffer::Update( uint vcount, void const* vertexData, uint vertexStride, buffer_attribute_t const* layout )
{
	m_layout = layout;
	m_elementStride = vertexStride;

	size_t dataBtyeSize = vcount * vertexStride;
	RenderBuffer::Update( vertexData, dataBtyeSize, vertexStride );
}
