#include "Texture.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//#define MSAA_ENABLED

Texture::Texture( const char* filePath, RenderContext* ctx, ID3D11Texture2D* handle )
{
	m_imageFilePath = filePath;
	m_owner = ctx;
	m_handle = handle;

	D3D11_TEXTURE2D_DESC desc;
	m_handle->GetDesc( &desc );
	m_dimensions.x = desc.Width;
	m_dimensions.y = desc.Height;
}

Texture::Texture( RenderContext* ctx, ID3D11Texture2D* handle )
	:m_owner( ctx ),
	m_handle( handle )
{
	D3D11_TEXTURE2D_DESC desc;
	handle->GetDesc(&desc);

	m_dimensions = IntVec2( desc.Width, desc.Height );
}

Texture::Texture( const Rgba8& color, RenderContext* ctx, ID3D11Texture2D* handle )
	:Texture( ctx, handle )
{
	unsigned char imageData[4] = { color.r, color.g, color.b, color.a };

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &imageData;
	initialData.SysMemPitch = 4;
	initialData.SysMemSlicePitch = 0;

	ID3D11Texture2D* texHandle = nullptr;
	ctx->m_device->CreateTexture2D( &desc, &initialData, &texHandle );

	m_dimensions = IntVec2( 1, 1 );
}

Texture::Texture( const Texture* copyFrom )
{
	if( !copyFrom->GetFilePath().empty() )
	{
		m_imageFilePath = copyFrom->GetFilePath().c_str();
	}

	m_owner = copyFrom->GetRenderContext();

	D3D11_TEXTURE2D_DESC desc;
	copyFrom->GetHandle()->GetDesc( &desc );
	desc.SampleDesc.Count = 1;		// maybe use msaa settings?
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;

	if( m_owner )
	{
		HRESULT result;
		result = m_owner->m_device->CreateTexture2D( &desc, NULL, &m_handle );
		GUARANTEE_OR_DIE( SUCCEEDED(result), "Failed on the copy constructor of texture class!" );												
	}
}

Texture::~Texture()
{
	delete m_renderTargetView;
	m_renderTargetView = nullptr;

	delete m_shaderResourceView;
	m_shaderResourceView = nullptr;


	DX_SAFE_RELEASE( m_depthStencilView );
	DX_SAFE_RELEASE( m_depthStencilBuffer );
	DX_SAFE_RELEASE( m_handle );
}

TextureView* Texture::GetRenderTargetView()
{
	if( m_renderTargetView )
	{
		return m_renderTargetView;
	}

	ID3D11Device* dev = m_owner->m_device;

	ID3D11RenderTargetView* rtv = nullptr;

#if defined( MSAA_ENABLED )
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory( &rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC) );
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	dev->CreateRenderTargetView( m_handle, &rtvDesc, &rtv );
#else 
	dev->CreateRenderTargetView( m_handle, nullptr, &rtv );
#endif

	if( rtv )
	{
		m_renderTargetView = new TextureView();
		m_renderTargetView->m_rtv = rtv;
	}

	return m_renderTargetView;
}

TextureView* Texture::GetOrCreateShaderResourceView()
{
	if( m_shaderResourceView ) {
		return m_shaderResourceView;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	m_handle->GetDesc( &texDesc );

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;	// Use all the MIP levels

	m_shaderResourceView = new TextureView();
	HRESULT hr = m_owner->m_device->CreateShaderResourceView( m_handle, &srvDesc, &m_shaderResourceView->m_srv );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Failed to create shader resources view!" );

	m_owner->m_context->GenerateMips( m_shaderResourceView->m_srv );

	return m_shaderResourceView;
}

ID3D11Texture2D* Texture::CreateDepthStencilBuffer()
{
	HRESULT result;
	D3D11_TEXTURE2D_DESC depthBufferDesc;

	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = g_theWindow->GetClientWidth();
	depthBufferDesc.Height = g_theWindow->GetClientHeight();
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_owner->m_device->CreateTexture2D( &depthBufferDesc, NULL, &m_owner->m_depthStencilBuffer );
	if( FAILED( result ) )
	{
		//ERROR_AND_DIE("depthBuffer create failed");
		//return false;
	}

	return m_handle;
}

ID3D11DepthStencilView* Texture::GetOrCreateDepthStencilView()
{
	if( m_depthStencilView != nullptr ) {
		return m_depthStencilView;
	}

	HRESULT result;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	// Initialize the depth stencil view.
	ZeroMemory( &depthStencilViewDesc, sizeof( depthStencilViewDesc ) );

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = /*DXGI_FORMAT_D24_UNORM_S8_UINT*/DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;


	// Create the depth stencil view.
	result = m_owner->m_device->CreateDepthStencilView( m_handle, &depthStencilViewDesc, &m_depthStencilView );
	if( FAILED( result ) )
	{
		return nullptr;
	}
	else {
		return m_depthStencilView;
	}
}

Texture* Texture::CreateDepthStencilBuffer( RenderContext* ctx, uint width, uint height )
{
	UNUSED( width );
	UNUSED( height );
	HRESULT result;
	D3D11_TEXTURE2D_DESC depthBufferDesc;

	ZeroMemory( &depthBufferDesc, sizeof( depthBufferDesc ) );

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = g_theWindow->GetClientWidth();
	depthBufferDesc.Height = g_theWindow->GetClientHeight();
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = /*DXGI_FORMAT_D24_UNORM_S8_UINT*/ DXGI_FORMAT_D32_FLOAT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	ID3D11Texture2D* texture2D = nullptr;

	result = ctx->m_device->CreateTexture2D( &depthBufferDesc, NULL, &texture2D );
	if( FAILED( result ) )
	{
		return nullptr;
	}
	else {
		return new Texture( ctx, texture2D );
	}
}

const float Texture::GetAspect() const
{
	return (float)m_dimensions.x / (float)m_dimensions.y;
}

Vec2 Texture::GetDimensions() const
{
	return Vec2( (float)m_dimensions.x, (float)m_dimensions.y );
}

IntVec2 Texture::GetTexelSize() const
{
	return IntVec2( (int)m_dimensions.x, (int)m_dimensions.y );
}

