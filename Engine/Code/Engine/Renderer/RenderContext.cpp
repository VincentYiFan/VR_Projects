#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Platform/Window.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ShaderState.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "openvr.h"

#pragma warning(push, 3)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning(pop)

#if !defined(WIN32_LEAN_AND_MEAN) 
#define WIN32_LEAN_AND_MEAN
#endif

#define INITGUID
#include <d3d11.h>  // d3d11 specific objects
#include <d3d11_1.h>
#include <dxgi.h>   // shared library used across multiple dx graphical interfaces
#include <dxgi1_2.h>
#include <dxgidebug.h>  // debug utility (mostly used for reporting and analytics)
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/Time.hpp"

#pragma comment( lib, "d3d11.lib" )        
#pragma comment( lib, "dxgi.lib" )         
#pragma comment( lib, "d3dcompiler.lib" )  

#define MSAA_ENABLED
//#define RENDER_DEBUG
//#define VR_ENABLED // #ToDo: Remove
//#define D3D11_DEBUG

// ASCII code from 0 to 127
constexpr auto MAX_CHAR = 128;

extern RenderContext* g_theRenderer;

void RenderContext::StartUp( Window* window )
{
	// Get the recommended size for the offscreen render target
#ifdef VR_ENABLED
	m_recommendedSize = g_gameConfigBlackboard.GetValue( "RecommendedRenderTargetSize", Vec2::ZERO );
	if( m_recommendedSize == Vec2::ZERO )
	{
		g_theConsole->Error( "%s - Recommended size for the offscreen render target failed to initialize" );
	}
#endif

	//IDXGISwapChain* pSwapChain = nullptr;
	//IDXGISwapChain1* pSwapChain1 = nullptr;

	//UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
	UINT flags = 0;
#if defined(RENDER_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create D3D11 Device
	HRESULT result;
	result = D3D11CreateDevice( nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&m_device,
		nullptr,
		&m_context );

	GUARANTEE_OR_DIE( SUCCEEDED( result ), "Fail to D3D11 Device" );

	//-------------------------------------------------------------------------------------------------------------
	IDXGIDevice* dxgiDevice = nullptr;
	result = m_device->QueryInterface( __uuidof(dxgiDevice), (void**)&dxgiDevice );
	IDXGIAdapter* dxgiAdapter = nullptr;
	result = dxgiDevice->GetParent( __uuidof(IDXGIAdapter), (void**)&dxgiAdapter );

	IDXGIFactory* dxgiFactory = nullptr;
	result = dxgiAdapter->GetParent( __uuidof(IDXGIFactory), (void**)&dxgiFactory  );

	// Create the swap chain
	IDXGISwapChain* swapchain;

	GUARANTEE_OR_DIE( m_msaa == 1 || m_msaa == 2 || m_msaa == 4 || m_msaa == 8 || m_msaa == 16, "Invalid xMSAA times!" );
	// Check If x4 MSAA Supported?
	result = ( m_device->CheckMultisampleQualityLevels( DXGI_FORMAT_R8G8B8A8_UNORM, m_msaa, &m_4xMsaaQuality ) );
	GUARANTEE_OR_DIE( m_4xMsaaQuality > 0, Stringf( "x%dMSAA is not supported!", m_msaa ) );

	// define the swap chain
	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	memset( &swapchainDesc, 0, sizeof( swapchainDesc ) );
	swapchainDesc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Flags = 0;
	HWND hwnd = (HWND)window->m_hwnd;
	swapchainDesc.OutputWindow = hwnd; // HWND for the window to be used
#if defined( MSAA_ENABLED )
	swapchainDesc.SampleDesc.Count = m_msaa;
	swapchainDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; 
#else
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // on swap, the old buffer is discarded
#endif
	swapchainDesc.Windowed = TRUE;
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferDesc.Width = window->GetClientWidth();
	swapchainDesc.BufferDesc.Height = window->GetClientHeight();
	result = dxgiFactory->CreateSwapChain( m_device, &swapchainDesc, &swapchain );

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	GUARANTEE_OR_DIE( SUCCEEDED( result ), "Fail to creating Render Context" );

	m_swapchain = new SwapChain( this, swapchain );
	m_defaultBackBuffer = m_swapchain->GetBackBuffer();
	m_defaultShader = GetOrCreateShader( "Data/Shaders/Default.hlsl" );
	CreateDepthStencilBuffer();
	CreateRasterState();

	m_frameUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_modelUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_tintUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_lightUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_materialUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );

	m_immediateMesh = new GPUMesh( this );
	m_defaultSampler = new Sampler( this, SAMPLER_ANISOTROPIC );
	//m_defaultSampler = new Sampler( this, SAMPLER_LINEAR );
	//m_defaultSampler = new Sampler( this, SAMPLER_POINT );
	//m_defaultSampler = new Sampler( this, SAMPLER_BILINEAR );

	m_defaultColorTex = CreateOrGetTextureFromFile( "Data/Textures/White.png" );

	m_effectCamera = new Camera();
	m_effectCamera->IntialUBO( this );
	m_effectCamera->SetClearMode( CLEAR_NONE, Rgba8::WHITE );

	CreateBlendState();
}	

void RenderContext::BeginFrame()
{
	
}

void RenderContext::UpdateFrameTime( float deltaSeconds )
{
	frame_data_t frameData;
	frameData.ststem_time = (float)GetCurrentTimeSeconds();
	frameData.system_delta_time = deltaSeconds;

	m_frameUBO->Update( &frameData, sizeof( frameData ), sizeof( frameData ) );
}

void RenderContext::UpdateTintColor( Rgba8 tint )
{
	tint_data_t tintData;
	tintData.tint[0] = (float)(tint.r / 255);
	tintData.tint[1] = (float)(tint.g / 255);
	tintData.tint[2] = (float)(tint.b / 255);
	tintData.tint[3] = (float)(tint.a / 255);

	m_tintUBO->Update( &tintData, sizeof( tintData ), sizeof( tintData ) );
	BindUniformBuffer( 3, m_tintUBO );
}

void RenderContext::UpdateImageEffect()
{
	m_materialUBO->Update( &m_imageEffectData, sizeof(m_imageEffectData), sizeof(m_imageEffectData) );
	BindUniformBuffer( 5, m_materialUBO );
}

void RenderContext::BindMaterialBuffer( RenderBuffer* ubo )
{
	BindUniformBuffer( 5, ubo );
}

//-------------------------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{
	m_swapchain->Present();
}

//-------------------------------------------------------------------------------------------------------------
void RenderContext::Shutdown()
{
	if( m_effectCamera != nullptr ) {
		m_effectCamera->CleanUBO();
		delete m_effectCamera;
		m_effectCamera = nullptr;
	}

	GUARANTEE_OR_DIE( m_renderTargetPool.size() == m_totalRenderTargetMade, "RenderTargetPool not fully released." );
	for( Texture* tex : m_renderTargetPool ) {
		delete tex;
	}
	m_renderTargetPool.clear();


	delete m_immediateMesh;
	m_immediateMesh = nullptr;

	delete m_swapchain;
	m_swapchain = nullptr;

	delete m_frameUBO;
	m_frameUBO = nullptr;

	delete m_modelUBO;
	m_modelUBO = nullptr;

	delete m_tintUBO;
	m_tintUBO = nullptr;

	delete m_lightUBO;
	m_lightUBO = nullptr;

	delete m_materialUBO;
	m_materialUBO = nullptr;

	delete m_frameColorTarget;
	m_frameColorTarget = nullptr;

	delete m_defaultSampler;
	m_defaultSampler = nullptr;

	delete m_indicesBuffer;
	m_indicesBuffer = nullptr;

	m_defaultBackBuffer = nullptr;
	m_lastBoundVBO = nullptr;

	for( int index = 0; index < (int) m_textureVector.size(); index++ ) {
		if( m_textureVector[index] != nullptr ) {
			delete m_textureVector[index];
			m_textureVector[index] = nullptr;
		}
	}

	for( int index = 0; index < (int)m_shaderVector.size(); index++ ){
		if( m_shaderVector[index] != nullptr ) {
			delete m_shaderVector[index];
			m_shaderVector[index] = nullptr;
		}
	}


	//DX_SAFE_RELEASE( m_lastVBO );
	DX_SAFE_RELEASE( m_alphaBlendState );
	DX_SAFE_RELEASE( m_additiveBlendState );
	DX_SAFE_RELEASE( m_opaqueBlendState );
	DX_SAFE_RELEASE( m_rasterState );
	DX_SAFE_RELEASE( m_context );
	DX_SAFE_RELEASE( m_context1 );
	DX_SAFE_RELEASE( m_depthStencilBuffer );
	DX_SAFE_RELEASE( m_depthStencilState );
	DX_SAFE_RELEASE( m_depthStencilView );

#if defined( D3D11_DEBUG )
	ID3D11Debug* d3dDebug;
	HRESULT hr = m_device->QueryInterface( __uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	if( SUCCEEDED( hr ) ) {
		hr = d3dDebug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL );
	}
	DX_SAFE_RELEASE( d3dDebug );
#endif

	DX_SAFE_RELEASE( m_device );
	DX_SAFE_RELEASE( m_device1 );
}

//-------------------------------------------------------------------------------------------------------------
void RenderContext::ClearScreen( const Rgba8& clearColor )
{
	float clearFloats[4];
	clearFloats[0] = (float) clearColor.r / 255.f;
	clearFloats[1] = (float) clearColor.g / 255.f;
	clearFloats[2] = (float) clearColor.b / 255.f;
	clearFloats[3] = (float) clearColor.a / 255.f;

	Texture* backbuffer = m_swapchain->GetBackBuffer();
	TextureView* backbuffer_rtv = backbuffer->GetRenderTargetView();

	ID3D11RenderTargetView* rtv = backbuffer_rtv->GetAsRTV();
	m_context->ClearRenderTargetView( rtv, clearFloats );
}

//-------------------------------------------------------------------------------------------------------------
void RenderContext::BeginCamera( const Camera& camera )
{
	Camera& m_currentCamera = const_cast<Camera&>( camera );
	m_currentCamera.UpdateUBO(); 
	m_isDrawing = true;

#if defined(RENDER_DEBUG)
	m_context->ClearState();
#endif

	Texture* colorTarget = camera.GetColorTarget();
	if( !colorTarget )
	{
		colorTarget = m_swapchain->GetBackBuffer();
	}

	std::vector<ID3D11RenderTargetView*> rtvs;
	int rtvCount = camera.GetColorTargetCount();
	rtvs.resize( rtvCount );

	if( rtvCount == 0 )
	{
		TextureView* view = colorTarget->GetRenderTargetView();
		ID3D11RenderTargetView* rtv = view->GetAsRTV();
		m_context->OMSetRenderTargets( 1, &rtv, nullptr );
	}
	else
	{
		for( int i = 0; i < rtvCount; ++i )
		{
			rtvs[i] = nullptr;

			Texture* theColorTarget = camera.GetColorTarget( i );
			if( theColorTarget != nullptr )
			{
				TextureView* rtv = theColorTarget->GetRenderTargetView();
				rtvs[i] = rtv->GetAsRTV();
			}
		}
	}
	

	IntVec2 outputSize = colorTarget->GetTexelSize();

	if( camera.m_canClearDepthBuffer ) 
	{
		m_context->OMSetRenderTargets( rtvCount, rtvs.data(), m_depthStencilView );
	}
	else 
	{	
		m_context->OMSetRenderTargets( rtvCount, rtvs.data(), nullptr );
	}


	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
#ifdef VR_ENABLED
	//m_recommendedSize = g_gameConfigBlackboard.GetValue( "RecommendedRenderTargetSize", Vec2::ZERO );
	viewport.Width = m_recommendedSize.x;
	viewport.Height = m_recommendedSize.y;
#else
	viewport.Width = (float)outputSize.x;	// texture->GetWidth();
	viewport.Height = (float)outputSize.y;	// texture->GetHeigth();
#endif
	
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;



	m_context->RSSetViewports( 1, &viewport );

	if( eCameraClearBit::CLEAR_COLOR_BIT & camera.m_clearMode ) 
	{
		Rgba8 clearColor = camera.GetClearColor();
		float clearFloats[4];
		clearFloats[0] = (float)clearColor.r / 255.f;
		clearFloats[1] = (float)clearColor.g / 255.f;
		clearFloats[2] = (float)clearColor.b / 255.f;
		clearFloats[3] = (float)clearColor.a / 255.f;

		//TextureView* backbuffer_rtv = colorTarget->GetRenderTargetView();
		//ID3D11RenderTargetView* rtv = backbuffer_rtv->GetAsRTV();

		for( int i = 0; i < rtvCount; ++i ) 
		{
			if( rtvs[i] != nullptr ) 
			{
				m_context->ClearRenderTargetView( rtvs[i], clearFloats );
			}
		}
	}

	m_lastVBO = nullptr;

	SetModelMatrix( Mat44::IDENTITY );
	UpdateTintColor( Rgba8::WHITE );
	UpdateLight();

	BindUniformBuffer( 0, m_frameUBO );
	BindUniformBuffer( 1, m_currentCamera.m_cameraUBO );

	BindShader( nullptr );
	BindTexture( nullptr );
	BindSampler( nullptr );

	SetBlendMode( BlendMode::ALPHA );
	// Clear Depth only the bool on camera is true
	if( m_currentCamera.m_canClearDepthBuffer )
	{
		m_context->ClearDepthStencilView( m_depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0 );
	}	

}

//-------------------------------------------------------------------------------------------------------------
void RenderContext::EndCamera( const Camera& camera )
{
	UNUSED( camera );
	DX_SAFE_RELEASE( m_depthStencilState );
}

void RenderContext::DrawVertexArray( int numVertexes, const Vertex_PCU* vertexArray )
{
	// Update a vertex buffer
	/*size_t bufferTotalByteSize = numVertexes * sizeof(Vertex_PCU);
	size_t elementSize = sizeof(Vertex_PCU);
	m_immediateVBO->Update( vertexArray, bufferTotalByteSize, elementSize );*/

	m_immediateMesh->UpdateVertices( numVertexes, vertexArray );
	m_immediateMesh->UpdateIndices( 0, nullptr );  // ClearIndices()

	DrawMesh( m_immediateMesh );
}

void RenderContext::DrawVertexArray( const std::vector<Vertex_PCU>& verts )
{
	int numVertexes = (int)verts.size();
	DrawVertexArray( numVertexes, &verts[0] );
}


void RenderContext::Draw( int numVertexes, int vertexOffset )
{
	FinalizeState();
	m_context->Draw( numVertexes, vertexOffset );
}

void RenderContext::DrawIndexed( int indexCount, int indexOffset, int vertexOffset )
{
	FinalizeState();

	m_context->DrawIndexed( indexCount, indexOffset, vertexOffset );
}

void RenderContext::DrawLine( const Vec2& start, const Vec2& end, const Rgba8& color, float thickness )
{
	Vec2 fwd = end - start;
	float r = 0.5f * thickness;
	fwd.SetLength(r);
	Vec2 Left  = fwd.GetRotated90Degrees();
	Vec2 Right = -Left;

	Vec2 endLeft   = end + fwd + Left;
	Vec2 endRight  = end + fwd + Right;
	Vec2 startLeft = start - fwd + Left;
	Vec2 startRight = start - fwd + Right;

	Vertex_PCU vertexesArray[6] = 
	{ 
		Vertex_PCU( Vec3(startRight.x, startRight.y, 0.f), color, Vec2(0.f,0.f)),
		Vertex_PCU( Vec3(endRight.x, endRight.y, 0.f), color, Vec2(0.f,0.f)),							
		Vertex_PCU( Vec3(endLeft.x, endLeft.y, 0.f), color, Vec2(0.f,0.f)),

		Vertex_PCU( Vec3(startRight.x, startRight.y, 0.f), color, Vec2(0.f,0.f)),
		Vertex_PCU( Vec3(endLeft.x, endLeft.y, 0.f), color, Vec2(0.f,0.f)),
		Vertex_PCU( Vec3(startLeft.x, startLeft.y, 0.f), color, Vec2(0.f,0.f))
	};

	DrawVertexArray( 6 , vertexesArray );
}

void RenderContext::DrawRing( const Vec2& center, float radius, const Rgba8& color, float thickness )
{
	constexpr float NUM_SIDES = 64.f;
	constexpr float degreesPerSide = 360.f / NUM_SIDES;
	for( int i = 0; i < 64; i++ )
	{
		thetaDegrees[i] = degreesPerSide * (float)i;
		innerCircleVertexes[i] = Vertex_PCU( Vec3( radius*CosDegrees( thetaDegrees[i] ), radius*SinDegrees( thetaDegrees[i] ), 0.f ), color, Vec2( 0.f, 0.f ) );
	}

	Vertex_PCU localInnerCircleVertexes[64] ={};
	for( int i = 0; i < 64; i++ )
	{
		localInnerCircleVertexes[i] = innerCircleVertexes[i];
	}
	TransfromVertexesArray( 64, localInnerCircleVertexes, 1.f, 0.f, center );
	for( int i = 0; i < 63; i++ )
	{
		g_theRenderer->DrawLine( Vec2( localInnerCircleVertexes[i].m_position.x, localInnerCircleVertexes[i].m_position.y ),
			Vec2( localInnerCircleVertexes[i+1].m_position.x, localInnerCircleVertexes[i+1].m_position.y ),
			color, thickness );
	}
	g_theRenderer->DrawLine( Vec2( localInnerCircleVertexes[63].m_position.x, localInnerCircleVertexes[63].m_position.y ),
		Vec2( localInnerCircleVertexes[0].m_position.x, localInnerCircleVertexes[0].m_position.y ),
		color, thickness );

}

void RenderContext::DrawAABB2( const AABB2& bounds, const Rgba8& tint )
{
	Vec2 LB( bounds.mins.x , bounds.mins.y );
	Vec2 RB( bounds.maxs.x , bounds.mins.y );
	Vec2 RT( bounds.maxs.x , bounds.maxs.y );
	Vec2 LT( bounds.mins.x , bounds.maxs.y );

	constexpr int NUM_VERTS = 6;
	Vertex_PCU verts[NUM_VERTS] =
	{
		Vertex_PCU( LB, tint, Vec2::ZERO ),
		Vertex_PCU( RB, tint, Vec2(1.f, 0.f) ),
		Vertex_PCU( RT, tint, Vec2(1.f, 1.f) ),
								  
		Vertex_PCU( LB, tint, Vec2::ZERO ),
		Vertex_PCU( RT, tint, Vec2(1.f, 1.f) ),
		Vertex_PCU( LT, tint, Vec2(0.f, 1.f) )
	};

	DrawVertexArray( NUM_VERTS, verts );
}

void RenderContext::DrawQuad( const AABB2& bounds, float z, const Rgba8& tint )
{
	Vec3 LB( bounds.mins.x, bounds.mins.y, z );
	Vec3 RB( bounds.maxs.x, bounds.mins.y, z );
	Vec3 RT( bounds.maxs.x, bounds.maxs.y, z );
	Vec3 LT( bounds.mins.x, bounds.maxs.y, z );

	constexpr int NUM_VERTS = 6;
	Vertex_PCU verts[NUM_VERTS] =
	{
		Vertex_PCU( LB, tint, Vec2::ZERO ),
		Vertex_PCU( RB, tint, Vec2( 1.f, 0.f ) ),
		Vertex_PCU( RT, tint, Vec2( 1.f, 1.f ) ),

		Vertex_PCU( LB, tint, Vec2::ZERO ),
		Vertex_PCU( RT, tint, Vec2( 1.f, 1.f ) ),
		Vertex_PCU( LT, tint, Vec2( 0.f, 1.f ) )
	};

	DrawVertexArray( NUM_VERTS, verts );
}

void RenderContext::DrawMesh( GPUMesh* mesh )
{
	BindVertexBuffer( mesh->GetVertexBuffer() );

	bool hasIndices = mesh->GetIndexCount() > 0;
	if( hasIndices ) 
	{
		BindIndexBuffer( mesh->GetIndexBuffer() );
		DrawIndexed( mesh->GetIndexCount(), 0, 0 );
	}
	else 
	{
		Draw( mesh->GetVertexCount(), 0 );
	}
}

void RenderContext::DrawOBB2( const OBB2& box, const Rgba8& tint )
{
	Vec2 center = box.GetCenter();
	Vec2 halfDimensions = box.m_halfDimensions;
	Vec2 iBasis = box.GetIBasisNormal();
	Vec2 jBasis = box.GetJBasisNormal();
	
	Vec2 BL = center - ( halfDimensions.x * iBasis ) - ( halfDimensions.y * jBasis );
	Vec2 BR = center + ( halfDimensions.x * iBasis ) - ( halfDimensions.y * jBasis );
	Vec2 TR = center + ( halfDimensions.x * iBasis ) + ( halfDimensions.y * jBasis );
	Vec2 TL = center - ( halfDimensions.x * iBasis ) + ( halfDimensions.y * jBasis );

	constexpr int NUM_VERTS = 6;
	Vertex_PCU verts[NUM_VERTS] = 
	{
		Vertex_PCU( BR, tint, Vec2::ZERO ),
		Vertex_PCU( TR, tint, Vec2::ZERO ),
		Vertex_PCU( BL, tint, Vec2::ZERO ),

		Vertex_PCU( BL, tint, Vec2::ZERO ),
		Vertex_PCU( TR, tint, Vec2::ZERO ),
		Vertex_PCU( TL, tint, Vec2::ZERO )
	};

	DrawVertexArray( NUM_VERTS, verts );
}

void RenderContext::DrawDisc( const Vec2& center, float radius, const Rgba8& tint )
{
	std::vector<Vertex_PCU> verts;

	constexpr float numSlices = 36.f; 
	const float perSliceDegrees = 360.f / numSlices;
	Vec2 DispForDrawingHalfCircle = Vec2( 1.f, 0.f ) * radius;
	float currentDegrees = 0.f;
	while( currentDegrees < 360.f )
	{
		verts.push_back( Vertex_PCU( center, tint, Vec2::ZERO ) );
		verts.push_back( Vertex_PCU( center + DispForDrawingHalfCircle, tint, Vec2::ZERO ) );
		DispForDrawingHalfCircle.RotateDegrees( perSliceDegrees );
		verts.push_back( Vertex_PCU( center + DispForDrawingHalfCircle, tint, Vec2::ZERO ) );
		currentDegrees += perSliceDegrees;
	}

	DrawVertexArray( verts );
}

void RenderContext::DrawCapsule( const Capsule2& capsule, const Rgba8& tint )
{
	std::vector<Vertex_PCU> verts;

	Vec2 center = 0.5f * (capsule.m_boneStart + capsule.m_boneEnd);
	Vec2 capsuleFwdDir = capsule.m_boneEnd - capsule.m_boneStart;
	Vec2 fullDimensions = Vec2( 2 * capsule.m_radius, GetDistance2D( capsule.m_boneStart, capsule.m_boneEnd ) );
	OBB2 box = OBB2( center, fullDimensions, Vec2( -capsuleFwdDir.y, capsuleFwdDir.x ).GetNormalized() );
	/*std::vector<Vec2> cornerPositions = box.GetFourCornerPositions();
	for( int index = 0; index < (int)cornerPositions.size(); index++ )
	{
		verts.push_back( Vertex_PCU( cornerPositions[index], tint, Vec2::ZERO ) );
	}*/

	DrawOBB2( box, tint );


	const float perSliceDegrees = 180.f / 36.f;
	//Vec2 point = capsule.m_boneStart - ( box.m_halfDimensions.x * box.m_iBasis );  
	Vec2 DispForDrawingHalfCircle = box.m_iBasis * capsule.m_radius;
	float currentDegrees = 0.f;
	while( currentDegrees < 180.f )
	{
		verts.push_back( Vertex_PCU( capsule.m_boneStart, tint, Vec2::ZERO ) );
		verts.push_back( Vertex_PCU( capsule.m_boneStart + DispForDrawingHalfCircle, tint, Vec2::ZERO ) );
		DispForDrawingHalfCircle.RotateDegrees( perSliceDegrees );
		verts.push_back( Vertex_PCU( capsule.m_boneStart + DispForDrawingHalfCircle, tint, Vec2::ZERO ) );
		currentDegrees += perSliceDegrees;
	}

	//Vec2 pointTwo = capsule.m_boneEnd + (box.m_halfDimensions.x * box.m_iBasis); 
	Vec2 DispForDrawingHalfCircleEnd = -box.m_iBasis * capsule.m_radius;
	float currentDegreesTwo = 0.f;
	while( currentDegreesTwo < 180.f )
	{
		verts.push_back( Vertex_PCU( capsule.m_boneEnd, tint, Vec2::ZERO ) );
		verts.push_back( Vertex_PCU( capsule.m_boneEnd + DispForDrawingHalfCircleEnd, tint, Vec2::ZERO ) );
		DispForDrawingHalfCircleEnd.RotateDegrees( perSliceDegrees );
		verts.push_back( Vertex_PCU( capsule.m_boneEnd + DispForDrawingHalfCircleEnd, tint, Vec2::ZERO ) );
		currentDegreesTwo += perSliceDegrees;
	}

	DrawVertexArray( verts );
}

void RenderContext::DrawPolygon( const std::vector<Vec2> points, const Rgba8& tint )
{
	int numPoints = (int)points.size();

	if( numPoints < 3 ) {
		return;
	}

	std::vector<Vertex_PCU> verts;

	for( int pointIndex = 1; pointIndex < numPoints - 1; pointIndex++ )
	{
		verts.push_back( Vertex_PCU( points[0],				 tint, Vec2::ZERO ) );
		verts.push_back( Vertex_PCU( points[pointIndex],	 tint, Vec2::ZERO ) );
		verts.push_back( Vertex_PCU( points[pointIndex + 1], tint, Vec2::ZERO ) );
	}

	DrawVertexArray( verts );
}

void RenderContext::BindShader( Shader* shader )
{
	ASSERT_OR_DIE( IsDrawing(), "ERROR" ); //  IsDrawing() -> DO I have a camera?

	Shader* tempShader = m_currentShader; // store the current shader

	m_currentShader = shader;
	if( m_currentShader == nullptr ) {
		m_currentShader = m_defaultShader;
	}

	if( m_currentShader != tempShader ) {
		m_shaderHasChanged = true; 
	}

	m_context->VSSetShader( m_currentShader->m_vertexStage.m_vs, nullptr, 0 );
	m_context->RSSetState( m_rasterState ); // Use default stage
	m_context->PSSetShader( m_currentShader->m_fragmentStage.m_fs, nullptr, 0 );
}

void RenderContext::BindShaderStateFromName( const char* shaderStateName )
{
	ShaderState* shaderState = GetShaderState( shaderStateName );
	BindShaderState( shaderState );
}

void RenderContext::BindShaderState( ShaderState* shaderState )
{
	BindShader( shaderState->m_shader );
	SetBlendMode( shaderState->m_blendMode );
	EnableDepth( shaderState->m_depthTest, shaderState->m_writeDepth );
	SetFrontCounterClockwise( shaderState->m_isWindingOrderCCW );
	SetFillMode( shaderState->m_fillMode );
	SetCullMode( shaderState->m_culling );
}

void RenderContext::BindShaderFromFile( char const* filename )
{
	Shader* shader = GetOrCreateShader( filename );

	BindShader( shader );
}

void RenderContext::BindMaterial( Material* material )
{
	BindShaderState( material->m_shaderState );

	// bind texture
	std::map<uint, Texture*>::iterator itTex;
	for( itTex = material->m_textureIdxPointer.begin(); itTex != material->m_textureIdxPointer.end(); ++itTex ) {
		uint slot = itTex->first;
		Texture* tex = itTex->second;
		BindTexture( tex, slot );
	}

	// bind sampler
	std::map<uint, Sampler*>::iterator itSamp;
	for( itSamp = material->m_samplerIdxPointer.begin(); itSamp != material->m_samplerIdxPointer.end(); ++itSamp ) {
		uint slot = itSamp->first;
		Sampler* sampler = itSamp->second;
		BindSampler( sampler, slot );
	}

	material->UpdateUBOIfDirty();
	BindMaterialBuffer( material->m_ubo );
}

void RenderContext::SetModelMatrix( Mat44 const& mat )
{
	transform_data_t transformData;
	transformData.matrix = mat;
	transformData.specular_factor = m_specularFactor;
	transformData.specular_power = m_specularPower;

	m_modelUBO->Update( &transformData, sizeof(transformData), sizeof(transformData) );
	BindUniformBuffer( 2, m_modelUBO );
}

Shader* RenderContext::GetOrCreateShader( const char* filename )
{
	for( int index = 0; index < (int)m_shaderVector.size(); index++ ) {
		if( m_shaderVector[index]->GetFilePath() == filename ) {
			return m_shaderVector[index];
		}
	}
	Shader* shader = new Shader( this );

	shader->CreateFromFile( filename );

	m_shaderVector.push_back( shader );

	return shader;
}

ShaderState* RenderContext::GetShaderState( const char* filename )
{
	ShaderState* shaderState = nullptr;

	std::map<std::string, ShaderState*>::iterator it;

	for( it = ShaderState::s_definitions.begin(); it != ShaderState::s_definitions.end(); it++ )
	{
		if( it->first == filename ) {
			shaderState = it->second;
		}
	}

	return shaderState;
}

Texture* RenderContext::CreateRenderTarget( IntVec2 texelSize )
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = texelSize.x;
	desc.Height = texelSize.y;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	
	ID3D11Texture2D* texHandle = nullptr;
	HRESULT hResult = m_device->CreateTexture2D( &desc, nullptr, &texHandle );
	if( FAILED( hResult ) )
	{
		ERROR_AND_DIE( "Failed on CreateRenderTarget!" );
	}

	Texture* texture = new Texture( this, texHandle );
	return texture;
}

Texture* RenderContext::AcquireRenderTargetMatching( Texture* texture )
{
	if( !texture ) {
		return nullptr;
	}

	IntVec2 size = texture->GetTexelSize();

	for( int i = 0; i < (int)m_renderTargetPool.size(); ++i )
	{
		Texture* rt = m_renderTargetPool[i];
		if( rt->GetTexelSize() == size )
		{
			// fast remove at index
			m_renderTargetPool[i] = m_renderTargetPool[m_renderTargetPool.size() - 1];
			m_renderTargetPool.pop_back();

			// return the object from the pool
			return rt;
		}
	}

	// nothing in the pool that matches - make a new one
	++m_totalRenderTargetMade;
	Texture* newRenderTarget = CreateRenderTarget( size );
	return newRenderTarget;
}

void RenderContext::ReleaseRenderTarget( Texture* texture )
{
	m_renderTargetPool.push_back( texture ); 
}

Texture* RenderContext::GetBackBuffer() const
{
	return m_swapchain->m_backbuffer;
}

void RenderContext::CopyTexture( Texture* dst, Texture* src )
{
	m_context->CopyResource( dst->GetHandle(), src->GetHandle() );
}

void RenderContext::StartEffecct( Texture* dst, Texture* src, Shader* shader )
{
	m_effectCamera->SetColorTarget( dst );

	BeginCamera( *m_effectCamera );
	BindShader( shader );
	BindTexture( src );

	SetBlendMode( BlendMode::_OPAQUE );
}

void RenderContext::EndEffect()
{
	m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	FinalizeState();
	m_context->Draw(3, 0);
	EndCamera( *m_effectCamera );
}


Texture* RenderContext::GetFrameColorTarget()
{
	return m_frameColorTarget;
}

void RenderContext::BindVertexBuffer( VertexBuffer* vbo )
{
	m_lastBoundVBO = vbo;
	
	ID3D11Buffer* vboHandle = vbo->m_handle;
	UINT stride = vbo->GetElementStride(); // how far from one vertex to next
	UINT offset = 0;

	if( m_lastVBO != vboHandle )
	{
		m_context->IASetVertexBuffers( 0, 1, &vboHandle, &stride, &offset );
		m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_lastVBO = vboHandle;
		m_currentLayout = vbo->GetLayout();
	}
}

void RenderContext::BindIndexBuffer( IndexBuffer* ibo )
{
	ID3D11Buffer* handle = nullptr;

	if( ibo != nullptr ) {
		handle = ibo->m_handle;
	}

	m_context->IASetIndexBuffer( handle, DXGI_FORMAT_R32_UINT, 0 );
}

void RenderContext::BindUniformBuffer( uint slot, RenderBuffer* ubo )
{
	ID3D11Buffer* uboHandle = ubo->m_handle;

	m_context->VSSetConstantBuffers( slot, 1, &uboHandle );
	m_context->PSSetConstantBuffers( slot, 1, &uboHandle );
}

void RenderContext::BindUniformBuffer( uint slot, const Camera* camera )
{
	ID3D11Buffer* uboHandle = camera->m_cameraUBO->m_handle;

	m_context->VSSetConstantBuffers( slot, 1, &uboHandle );
	m_context->PSSetConstantBuffers( slot, 1, &uboHandle );
}


void RenderContext::BindTexture( const Texture* constTex )
{
	Texture* texture = const_cast<Texture*>( constTex );
	if( texture == nullptr ) {
		texture = m_defaultColorTex;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->GetAsSRV();
	m_context->PSSetShaderResources( 0, 1, &srvHandle );
}

void RenderContext::BindTexture( const Texture* constTex, const int slot )
{
	Texture* texture = const_cast<Texture*>(constTex);
	if( texture == nullptr ) {
		texture = m_defaultColorTex;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->GetAsSRV();
	m_context->PSSetShaderResources( slot, 1, &srvHandle );
}

void RenderContext::BindNormalTexture( const Texture* constTex )
{
	Texture* texture = const_cast<Texture*>(constTex);
	if( texture == nullptr ) {
		texture = m_defaultColorTex;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->GetAsSRV();
	m_context->PSSetShaderResources( 1, 1, &srvHandle );
}

void RenderContext::BindSpecularTexture( const Texture* constTex )
{
	Texture* texture = const_cast<Texture*>(constTex);
	if( texture == nullptr ) {
		texture = m_defaultColorTex;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->GetAsSRV();
	m_context->PSSetShaderResources( 2, 1, &srvHandle );
}

void RenderContext::BindPatternTexture( const Texture* constTex )
{
	Texture* texture = const_cast<Texture*>(constTex);
	if( texture == nullptr ) {
		texture = m_defaultColorTex;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->GetAsSRV();
	m_context->PSSetShaderResources( 8, 1, &srvHandle );
}

void RenderContext::BindSampler( Sampler* sampler )
{
	if( sampler == nullptr ) {
		sampler = m_defaultSampler;
	}

	ID3D11SamplerState* samplerHandle = sampler->GetHandle();
	m_context->PSSetSamplers( 0, 1, &samplerHandle );
}

void RenderContext::BindSampler( Sampler* sampler, const int slot )
{
	if( sampler == nullptr ) {
		sampler = m_defaultSampler;
	}

	ID3D11SamplerState* samplerHandle = sampler->GetHandle();
	m_context->PSSetSamplers( slot, 1, &samplerHandle );
}

Texture* RenderContext::CreateTextureFromFile( const char* filePath )
{
	//unsigned int textureID = 0;
	int imageTexelSizeX = 0; // This will be filled in for us to indicate image width
	int imageTexelSizeY = 0; // This will be filled in for us to indicate image height
	int numComponents = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 4; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* imageData = stbi_load( filePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );

	// Check if the load was successful
	GUARANTEE_OR_DIE( imageData, Stringf( "Failed to load image \"%s\"", filePath ) );
	GUARANTEE_OR_DIE( numComponents >= 3 && numComponents <= 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", filePath, numComponents, imageTexelSizeX, imageTexelSizeY ) );
	
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = imageTexelSizeX;
	desc.Height = imageTexelSizeY;
	desc.MipLevels = 0;		// We want all the mipmap levels down to 1x1
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;	// GPU needs to be able to write back to this texture, the texture needs to be final output of the pipeline
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	//D3D11_SUBRESOURCE_DATA initialData;
	//initialData.pSysMem = imageData;
	//initialData.SysMemPitch = imageTexelSizeX * 4;
	//initialData.SysMemSlicePitch = 0;

	ID3D11Texture2D* texHandle = nullptr;
	//m_device->CreateTexture2D( &desc, &initialData, &texHandle );
	m_device->CreateTexture2D( &desc, nullptr, &texHandle );
	GUARANTEE_OR_DIE( texHandle, "RenderContext::CreateTextureFromFile failed to setup texHandle" );

	// write image data into mip level
	m_context->UpdateSubresource( texHandle, 0u, nullptr, imageData, imageTexelSizeX * 4, 0u );

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free( imageData );

	Texture* texture = new Texture( filePath, this, texHandle );
	m_textureVector.push_back( texture );
	return texture;
}



Texture* RenderContext::CreateTextureFromColor( Rgba8 color )
{
	return m_defaultColorTex;
}

void RenderContext::FinalizeState()
{
	if( m_lastBoundLayout != m_currentLayout || m_shaderHasChanged )
	{
		ID3D11InputLayout* layout = m_currentShader->GetOrCreateInputLayout( m_currentLayout );
		m_context->IASetInputLayout( layout );

		m_lastBoundLayout = m_currentLayout;
		m_shaderHasChanged = false;
	}
}

bool RenderContext::CreateDepthStencilBuffer()
{
	HRESULT result;
	D3D11_TEXTURE2D_DESC depthBufferDesc;

	ZeroMemory( &depthBufferDesc, sizeof( depthBufferDesc ) );

	// Set up the description of the depth buffer.
	
#ifdef VR_ENABLED
	//m_recommendedSize = g_gameConfigBlackboard.GetValue( "RecommendedRenderTargetSize", Vec2::ZERO );
	depthBufferDesc.Width = (UINT)m_recommendedSize.x;
	depthBufferDesc.Height = (UINT)m_recommendedSize.y;
#else
	depthBufferDesc.Width = g_theWindow->GetClientWidth();
	depthBufferDesc.Height = g_theWindow->GetClientHeight();
#endif
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
#if defined( MSAA_ENABLED )
	depthBufferDesc.SampleDesc.Count = m_msaa;
	depthBufferDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
#else
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
#endif
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_device->CreateTexture2D( &depthBufferDesc, NULL, &m_depthStencilBuffer );
	if( FAILED( result ) )
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	// Initialize the description of the stencil state.
	ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_device->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilState );
	if( FAILED( result ) )
	{
		return false;
	}

	// Bind the depth-stencil state.
	m_context->OMSetDepthStencilState( m_depthStencilState, 1 );


	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	// Initialize the depth stencil view.
	ZeroMemory( &depthStencilViewDesc, sizeof( depthStencilViewDesc ) );

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
#if defined( MSAA_ENABLED )
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
#else
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
#endif
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = m_device->CreateDepthStencilView( m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView );
	if( FAILED( result ) )
	{
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	//m_context->OMSetRenderTargets( 1, &m_renderTargetView, m_depthStencilView );

	return true;
}

BitmapFont* RenderContext::CreateBitmapFontFromFile( const char* FontFilePath )
{
	//BitmapFont* m_bitmapFont;
	std::string path = Stringf( "%s.png" ,FontFilePath );
	Texture* textTexture = CreateOrGetTextureFromFile( path.c_str() );
	BitmapFont* bitmapFont = new BitmapFont( FontFilePath, textTexture );
	m_loadedFonts.push_back( bitmapFont );
	return bitmapFont;
}

BitmapFont* RenderContext::CreateOrGetBitmapFont( const char* bitmapFontFilePathNoExtension )
{
	for( int index = 0; index < (int)m_loadedFonts.size(); index++ )
	{
		if( m_loadedFonts[index]->m_fontName == bitmapFontFilePathNoExtension )
			return m_loadedFonts[index];			
	}
	return CreateBitmapFontFromFile( bitmapFontFilePathNoExtension );
}

Vec3 RenderContext::ConvertRgba8ToVec3( Rgba8 rgba8 ) const
{
	Vec3 vec3;
	vec3.x = (float)(rgba8.r / 255);
	vec3.y = (float)(rgba8.g / 255);
	vec3.z = (float)(rgba8.b / 255);
	//finalColor.w = (float)(color.a / 255);
	return vec3;
}

void RenderContext::EnableDepth( eCompareOp op, bool write )
{
	//ASSERT_OR_DIE( m_currentCamera != nullptr, "Only call between BeginCamera and EndCamera" );

	// ID3D11DepthStencilState* m_depthStencilState = nullptr; 
	if( m_depthStencilState != nullptr ) {
		DX_SAFE_RELEASE( m_depthStencilState );
	}

	D3D11_DEPTH_STENCIL_DESC desc;
	memset( &desc, 0, sizeof( desc ) ); 

	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	switch( op )
	{
	case eCompareOp::COMPARE_FUNC_NEVER:			desc.DepthFunc = D3D11_COMPARISON_NEVER;			break;
	case eCompareOp::COMPARE_FUNC_LESS:				desc.DepthFunc = D3D11_COMPARISON_LESS;				break;
	case eCompareOp::COMPARE_FUNC_EQUAL:			desc.DepthFunc = D3D11_COMPARISON_EQUAL;			break;
	case eCompareOp::COMPARE_FUNC_LEQUAL:			desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;		break;
	case eCompareOp::COMPARE_FUNC_GREATER:			desc.DepthFunc = D3D11_COMPARISON_GREATER;			break;
	case eCompareOp::COMPARE_FUNC_NOT_EQUAL:		desc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;		break;
	case eCompareOp::COMPARE_FUNC_GEQUAL:			desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;	break;
	case eCompareOp::COMPARE_FUNC_ALWAYS:			desc.DepthFunc = D3D11_COMPARISON_ALWAYS;			break;
	default:
		break;
	}

	m_device->CreateDepthStencilState( &desc, &m_depthStencilState );
	m_context->OMSetDepthStencilState( m_depthStencilState, 0 );
}

void RenderContext::DisableDepth() 
{
	EnableDepth( eCompareOp::COMPARE_FUNC_ALWAYS, false );
}

void RenderContext::ClearDepth( Texture* depthStencilTexture, float depth, uint stencil )
{
	TextureView* view = depthStencilTexture->GetOrCreateShaderResourceView();
	ID3D11DepthStencilView* dsv = view->GetAsDSV();
	m_context->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH, depth, (UINT8)stencil ); 
}

Texture* RenderContext::CreateOrGetTextureFromFile( const char* imageFileType )
{
	for( int index = 0; index < m_textureVector.size(); index++ ) {
		if( m_textureVector[index]->GetFilePath() == imageFileType ) {
			return m_textureVector[index];
		}
	}
	return CreateTextureFromFile( imageFileType );
}

void RenderContext::CreateBlendState()
{
	D3D11_BLEND_DESC alphaDesc;
	alphaDesc.AlphaToCoverageEnable = false;
	alphaDesc.IndependentBlendEnable = false;
	alphaDesc.RenderTarget[0].BlendEnable = true;
	alphaDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	alphaDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	alphaDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	alphaDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	alphaDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	alphaDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &alphaDesc, &m_alphaBlendState );



	D3D11_BLEND_DESC additiveDesc;
	additiveDesc.AlphaToCoverageEnable = false;
	additiveDesc.IndependentBlendEnable = false;
	additiveDesc.RenderTarget[0].BlendEnable = true;
	additiveDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	additiveDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	additiveDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	additiveDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	additiveDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	additiveDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	additiveDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &additiveDesc, &m_additiveBlendState );



	D3D11_BLEND_DESC opaqueDesc;
	opaqueDesc.AlphaToCoverageEnable = false;
	opaqueDesc.IndependentBlendEnable = false;
	opaqueDesc.RenderTarget[0].BlendEnable = false;
	opaqueDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	opaqueDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	opaqueDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;

	opaqueDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	opaqueDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	opaqueDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	opaqueDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &opaqueDesc, &m_opaqueBlendState );
}

void RenderContext::CreateRasterState()
{
	D3D11_RASTERIZER_DESC desc;

	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_NONE; // Never throw away triangle, every triangle show on the screen
	desc.FrontCounterClockwise = TRUE; // the only reason we're doing this; 
	desc.DepthBias = 0U;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = TRUE;
	desc.ScissorEnable = FALSE;
#if defined ( MSAA_ENABLED )
	desc.MultisampleEnable = TRUE;
	desc.AntialiasedLineEnable = TRUE;
#else
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;
#endif
	
	m_device->CreateRasterizerState( &desc, &m_rasterState );
}

void RenderContext::SetCullMode( D3D11_CULL_MODE newCullMode )
{
	if( m_rasterState == nullptr ) {
		ERROR_AND_DIE( "Failed to set rasterizer state: The rasterizer state is nullptr." );
	}

	D3D11_RASTERIZER_DESC desc;
	m_rasterState->GetDesc( &desc );
	if( desc.CullMode != newCullMode )
	{
		desc.CullMode = newCullMode;
	}

	ID3D11RasterizerState* tempRasterState;
	m_device->CreateRasterizerState( &desc, &tempRasterState );
	m_context->RSSetState( tempRasterState );

	DX_SAFE_RELEASE( m_rasterState );
	m_rasterState = tempRasterState;

	/*m_device->CreateRasterizerState( &desc, &m_rasterState );*/
}

void RenderContext::SetFillMode( D3D11_FILL_MODE newFillMode )
{
	if( m_rasterState == nullptr ) {
		ERROR_AND_DIE( "Failed to set rasterizer state: The rasterizer state is nullptr." );
	}

	D3D11_RASTERIZER_DESC desc;
	m_rasterState->GetDesc( &desc );

	if( desc.FillMode != newFillMode ) {
		desc.FillMode = newFillMode;
	}

	ID3D11RasterizerState* tempRasterState;
	m_device->CreateRasterizerState( &desc, &tempRasterState );
	m_context->RSSetState( tempRasterState );

	DX_SAFE_RELEASE( m_rasterState );
	m_rasterState = tempRasterState;
}

void RenderContext::SetFrontCounterClockwise( bool isCCW )
{
	if( m_rasterState == nullptr ) {
		ERROR_AND_DIE( "Failed to set rasterizer state: The rasterizer state is nullptr." );
	}

	D3D11_RASTERIZER_DESC desc;
	m_rasterState->GetDesc( &desc );

	if( isCCW ) {
		desc.FrontCounterClockwise = TRUE;
	}
	else {
		desc.FrontCounterClockwise = FALSE;
	}
	ID3D11RasterizerState* tempRasterState;
	m_device->CreateRasterizerState( &desc, &tempRasterState );
	m_context->RSSetState( tempRasterState );

	DX_SAFE_RELEASE( m_rasterState );
	m_rasterState = tempRasterState;
}


void RenderContext::UpdateLight()
{
	//light_constants_t light_data_t;
	//light_data_t.ambient = m_ambientLightData;
	//light_data_t.light = m_lightData;
	
	//m_lightUBO->Update( &light_data_t, sizeof( light_data_t ), sizeof( light_data_t ) );
	m_lightUBO->Update( &m_lightConstants, sizeof( m_lightConstants ), sizeof( m_lightConstants ) );
	BindUniformBuffer( 4, m_lightUBO );
}

void RenderContext::SetAmbientColor( Rgba8 color )
{
	//m_ambientLightData.x = (float)(color.r / 255);
	//m_ambientLightData.y = (float)(color.g / 255);
	//m_ambientLightData.z = (float)(color.b / 255);
	m_lightConstants.ambient.x= (float)(color.r / 255);
	m_lightConstants.ambient.y= (float)(color.g / 255);
	m_lightConstants.ambient.z= (float)(color.b / 255);
}

void RenderContext::SetAmbientIntensity( float intensity )
{
	m_lightConstants.ambient.w = intensity;
	// mark light as Dirty();
}

void RenderContext::SetAmbientLight( Rgba8 color, float intensity )
{
	SetAmbientColor( color );
	SetAmbientIntensity( intensity );
}

void RenderContext::EnableLight( uint idx, light_t lightInfo )
{
	m_lightConstants.lights[idx] = lightInfo;
}

void RenderContext::DisableLight( uint idx )
{
	m_lightConstants.lights[idx].intensity = 0.f;
}

void RenderContext::EnableFog( float nearFog, float farFog, Rgba8 nearFogColor, Rgba8 farFogColor )
{
	m_lightConstants.fog_near_distance = nearFog;
	m_lightConstants.fog_far_distance = farFog;
	m_lightConstants.fog_near_color = ConvertRgba8ToVec3( nearFogColor );
	m_lightConstants.fog_far_color = ConvertRgba8ToVec3( farFogColor );
}

void RenderContext::DisableFog()
{
	m_lightConstants.fog_near_distance = m_lightConstants.fog_far_distance;
}

void RenderContext::SetShaderMode( eShaderMode mode )
{
	switch( mode ) 
	{
	case eShaderMode::SHADER_LIT:
	{
		BindTexture( CreateOrGetTextureFromFile( "Data/Textures/example_colour.png" ) );
		BindNormalTexture( CreateOrGetTextureFromFile( "Data/Textures/example_normal.png" ) );
		BindSpecularTexture( CreateOrGetTextureFromFile( "Data/Textures/example_normal.png" ) );
		BindShader( GetOrCreateShader( "Data/Shaders/Lit.hlsl" ) );
		//BindPatternTexture( CreateOrGetTextureFromFile( "Data/Textures/noise.png" ) );
	} break;

	case eShaderMode::SHADER_DISSOLVE:
	{
		BindTexture( CreateOrGetTextureFromFile( "Data/Textures/example_colour.png" ) );
		BindNormalTexture( CreateOrGetTextureFromFile( "Data/Textures/example_normal.png" ) );
		BindSpecularTexture( CreateOrGetTextureFromFile( "Data/Textures/example_normal.png" ) );
		BindShader( GetOrCreateShader( "Data/Shaders/Dissolve.hlsl" ) );
		BindPatternTexture( CreateOrGetTextureFromFile ("Data/Textures/noise.png" ) );
	} break;

	case eShaderMode::SHADER_TANGENT:
	{
		BindShader( GetOrCreateShader( "Data/Shaders/Tangent.hlsl" ) );
	} break;

	case eShaderMode::SHADER_BITANGENT:
	{
		BindShader( GetOrCreateShader( "Data/Shaders/Bitangent.hlsl" ) );
	} break;

	case eShaderMode::SHADER_NORMAL:
	{
		BindShader( GetOrCreateShader( "Data/Shaders/Normal.hlsl" ) );
		//BindTexture( CreateOrGetTextureFromFile( "Data/Textures/example_colour.png" ) );
		BindNormalTexture( CreateOrGetTextureFromFile( "Data/Textures/example_normal.png" ) );
	} break;

	case eShaderMode::SHADER_SURFACE_NORMAL:
	{
		BindShader( GetOrCreateShader( "Data/Shaders/SurfaceNormal.hlsl" ) );
		//BindTexture( CreateOrGetTextureFromFile( "Data/Textures/example_colour.png" ) );
		BindNormalTexture( CreateOrGetTextureFromFile( "Data/Textures/example_normal.png" ) );
	} break;

	case eShaderMode::SHADER_PARALLAX:
	{
		BindShader( GetOrCreateShader( "Data/Shaders/Parallax.hlsl" ) );
		BindTexture( CreateOrGetTextureFromFile( "Data/Textures/ruin_wall_03_0.png" ) );
		BindTexture( CreateOrGetTextureFromFile( "Data/Textures/ruin_wall_03_height_0.png" ), 9 );
		BindNormalTexture( CreateOrGetTextureFromFile( "Data/Textures/ruin_wall_03_norm_0.png" ) );
		BindSpecularTexture( nullptr );
	}
	}
}

std::string RenderContext::GetShaderModeName( eShaderMode mode ) const
{
	std::string text;
	switch( mode ) {
	case eShaderMode::SHADER_LIT:				return "Lit.hlsl";				break;
	case eShaderMode::SHADER_DISSOLVE:			return "Dissolve.hlsl";			break;
	case eShaderMode::SHADER_TANGENT:			return "Tangent.hlsl";			break;
	case eShaderMode::SHADER_BITANGENT:			return "Bitangent.hlsl";		break;
	case eShaderMode::SHADER_NORMAL:			return "Normal.hlsl";			break;
	case eShaderMode::SHADER_SURFACE_NORMAL:	return "SurfaceNormal.hlsl";	break;
	//case eShaderMode::SHADER_TRIPLANAR:			return "Triplanar.hlsl";		break;
	case eShaderMode::SHADER_PARALLAX:			return "Parallax.hlsl";			break;

	default: ERROR_AND_DIE( "RenderContext: GetShaderModeName() can't find the correct name" ); }
}

void RenderContext::SetLightAttenuation( Vec3 att, int idx )
{
	m_lightConstants.lights[idx].attenuation = att;
}

Vec3 RenderContext::GetAttenuationFromAttMode( eAttenuationMode mode ) const
{
	switch( mode ) {
	case eAttenuationMode::ATTENUATION_LINEAR:		return Vec3( 0.f, 1.f, 0.f ); break;
	case eAttenuationMode::ATTENUATION_QUADRATIC:	return Vec3( 0.f, 0.f, 1.f ); break;
	case eAttenuationMode::ATTENUATION_CONSTANT:	return Vec3( 1.f, 0.f, 0.f ); break;

	default: ERROR_AND_DIE( "RenderContext: GetAttenuationFromAttMode() can't find the mode" );	}
}

eAttenuationMode RenderContext::CovertAttToEnumAttMode( const Vec3 attenaution ) const
{
	eAttenuationMode mode = eAttenuationMode::ATTENUATION_QUADRATIC;

	if( attenaution == Vec3( 0.f, 1.f, 0.f ) ) { mode = eAttenuationMode::ATTENUATION_LINEAR; }
	if( attenaution == Vec3( 0.f, 0.f, 1.f ) ) { mode = eAttenuationMode::ATTENUATION_QUADRATIC; }
	if( attenaution == Vec3( 1.f, 0.f, 0.f ) ) { mode = eAttenuationMode::ATTENUATION_CONSTANT; }

	return mode;
}

std::string RenderContext::GetAttenuationAsString( eAttenuationMode mode ) const
{
	switch( mode ) {
	case eAttenuationMode::ATTENUATION_LINEAR:		return "Linear";	break;
	case eAttenuationMode::ATTENUATION_QUADRATIC:	return "Quadratic"; break;
	case eAttenuationMode::ATTENUATION_CONSTANT:	return "Constant";	break;

	default: ERROR_AND_DIE( "RenderContext: GetAttenuationAsString() can't find the mode" );
	}
}

void RenderContext::SetSpecularFactor( const float specularFactor )
{
	m_specularFactor = specularFactor;
}

void RenderContext::SetSpecularPower( const float specularPower )
{
	m_specularPower = specularPower;
}


void RenderContext::SetBlendMode( BlendMode blendMode )
{
	float const zeros[] = { 0.f, 0.f, 0.f, 0.f };

	switch( blendMode )
	{
	case BlendMode::ALPHA:		m_context->OMSetBlendState( m_alphaBlendState, zeros, ~(UINT)0 );		break;
	case BlendMode::ADDITIVE:	m_context->OMSetBlendState( m_additiveBlendState, zeros, ~(UINT)0 );	break;
	case BlendMode::_OPAQUE:	m_context->OMSetBlendState( m_opaqueBlendState, zeros, ~(UINT)0 );		break;
	}
}


