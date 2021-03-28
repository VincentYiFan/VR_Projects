#include "Engine/Renderer/IndexBuffer.hpp"

IndexBuffer::IndexBuffer( RenderContext* ctx, eRenderMemoryHint hint )
	:RenderBuffer( ctx, INDEX_BUFFER_BIT, hint )
{
}

void IndexBuffer::Update( uint icount, uint const* indices )
{
	size_t elementByteSize = sizeof( uint );
	size_t dataBtyeSize = icount * elementByteSize;

	RenderBuffer::Update( indices, dataBtyeSize, elementByteSize );
}

void IndexBuffer::Update( std::vector<uint> const& indices )
{
	Update( (uint)indices.size(), &indices[0] );
}

HRESULT IndexBuffer::Initialize( ID3D11Device* device, uint* data, UINT numIndices )
{
	//this->m_bufferByteSize = numIndices; // #SD2ToDo: Make sure this is correct.
	//Load Index Data
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory( &indexBufferDesc, sizeof( indexBufferDesc ) );
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof( uint ) * numIndices;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	indexBufferData.pSysMem = data;
	HRESULT hr = device->CreateBuffer( &indexBufferDesc, &indexBufferData, &m_handle );
	return hr;
}
