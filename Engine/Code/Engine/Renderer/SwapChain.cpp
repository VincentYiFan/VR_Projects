#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/Texture.hpp"

SwapChain::SwapChain( RenderContext* owner, IDXGISwapChain* handle )
	:m_owner(owner),
	m_handle(handle)
{
}

SwapChain::~SwapChain()
{
	delete m_backbuffer;
	m_backbuffer = nullptr;

	DX_SAFE_RELEASE(m_handle);
	m_owner = nullptr;
}

void SwapChain::Present( int vsync )
{
	m_handle->Present( vsync, 0 );
}

Texture* SwapChain::GetBackBuffer()
{
	if( nullptr!= m_backbuffer )
	{
		return m_backbuffer;
	}

	ID3D11Texture2D* texHandle = nullptr;
	m_handle->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&texHandle );

	m_backbuffer = new Texture( m_owner, texHandle );

	return m_backbuffer;
}
