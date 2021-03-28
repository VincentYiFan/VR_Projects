#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Platform/Window.hpp"
#include <string>

struct Vec2;
struct Rgba8;
class RenderContext;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;

typedef unsigned int uint; 

extern Window* g_theWindow;


class Texture {

public:
	Texture( const char* filePath, RenderContext* ctx, ID3D11Texture2D* handle );
	Texture( RenderContext* ctx, ID3D11Texture2D* handle );
	Texture( const Rgba8& color ,RenderContext* ctx, ID3D11Texture2D* handle );
	//Texture( const Texture* copyFrom, bool multiSample = false );
	Texture( const Texture* copyFrom );

	~Texture();

	TextureView* GetRenderTargetView();
	TextureView* GetOrCreateShaderResourceView();

	ID3D11Texture2D* CreateDepthStencilBuffer();
	ID3D11DepthStencilView* GetOrCreateDepthStencilView();

	static Texture* CreateDepthStencilBuffer( RenderContext* ctx, uint width, uint height );

	RenderContext* GetRenderContext() const { return m_owner; }

	int GetTextureID() const { return m_textureID; }
	std::string const& GetFilePath() const { return m_imageFilePath; }
	const float GetAspect() const;

	Vec2 GetDimensions() const;
	IntVec2 GetTexelSize() const;
	ID3D11Texture2D* GetHandle() const { return m_handle; }

private:
	RenderContext* m_owner = nullptr;
	ID3D11Texture2D* m_handle = nullptr;

	unsigned int m_textureID = 0;
	std::string m_imageFilePath;

	TextureView* m_renderTargetView = nullptr;
	TextureView* m_shaderResourceView = nullptr;

	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11Texture2D* m_depthStencilBuffer;

	IntVec2		m_dimensions = IntVec2(0,0);
};