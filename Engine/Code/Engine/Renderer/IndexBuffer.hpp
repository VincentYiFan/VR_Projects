#pragma once
#include "Engine/Renderer/RenderBuffer.hpp"
#include "D3D11Common.hpp"
#include <vector>

class IndexBuffer : public RenderBuffer 
{
public:
	IndexBuffer( RenderContext* ctx, eRenderMemoryHint hint );

	void Update( uint icount, uint const* indices );
	void Update( std::vector<uint> const& indices ); // helper, calls one above

	HRESULT Initialize( ID3D11Device* device, uint* data, UINT numIndices );
};
