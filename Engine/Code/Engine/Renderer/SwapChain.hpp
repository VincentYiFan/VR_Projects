#pragma once

class RenderContext;
struct IDXGISwapChain;
class Texture;

class SwapChain
{
public:
	SwapChain( RenderContext* owner, IDXGISwapChain* handle );
	~SwapChain();

	void Present( int vsync = 0 );

	Texture* GetBackBuffer();

public:
	RenderContext* m_owner;		// (which context was the creator)
	IDXGISwapChain* m_handle;  

	Texture* m_backbuffer = nullptr;
};
