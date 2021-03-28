#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Platform/Window.hpp"
#include <vector>

typedef unsigned int uint;

//-------------------------------------------------------------------------------------------------------------------------------------------------
struct ID3D11Device;
struct ID3D11Device1;
struct ID3D11DeviceContext;
struct ID3D11DeviceContext1;
struct ID3D11Buffer;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
enum D3D11_CULL_MODE : int;
enum D3D11_FILL_MODE : int;
//-------------------------------------------------------------------------------------------------------------------------------------------------
class Shader;
class ShaderState;
class Material;
class RenderBuffer;
class VertexBuffer;
class IndexBuffer;
class Sampler;
class GPUMesh;
//-------------------------------------------------------------------------------------------------------------------------------------------------

enum class eCompareOp
{
	COMPARE_FUNC_NEVER,           // D3D11_COMPARISON_NEVER
	COMPARE_FUNC_ALWAYS,          // D3D11_COMPARISON_ALWAYS
	COMPARE_FUNC_LEQUAL,          // D3D11_COMPARISON_LESS_EQUAL
	COMPARE_FUNC_GEQUAL,          // D3D11_COMPARISON_GREATER_EQUAL
	COMPARE_FUNC_EQUAL,
	COMPARE_FUNC_GREATER,
	COMPARE_FUNC_LESS,
	COMPARE_FUNC_NOT_EQUAL,
};

enum class eAttenuationMode
{
	ATTENUATION_LINEAR,
	ATTENUATION_QUADRATIC,
	ATTENUATION_CONSTANT,

	NUM_ATTENUATION_MODES
};

enum class eShaderMode
{
	SHADER_LIT,
	SHADER_DISSOLVE,
	SHADER_TANGENT,
	SHADER_BITANGENT,
	SHADER_NORMAL,
	SHADER_SURFACE_NORMAL,
	//SHADER_TRIPLANAR,
	SHADER_PARALLAX,

	NUM_SHADER_MODES
};

enum class BlendMode
{
	DEFAULT = -1,

	ALPHA,
	ADDITIVE,
	_OPAQUE
};

enum eBufferSlot
{
	UBO_FRAME_SLOT = 0,
	UBO_CAMERA_SLOT = 1,
};

struct frame_data_t
{
	float ststem_time;
	float system_delta_time;

	float padding[2];
};

struct transform_data_t
{
	Mat44 matrix;

	float specular_factor = 1.f;
	float specular_power = 32.f;
	float pad00[2];

};

struct material_data_t
{
	Vec3 color;
	float fresnel_power;

	Vec3 burn_start_color = Vec3( 1.f, 0.f, 0.f );
	float burn_edge_width = 0.1f;

	Vec3 burn_end_color = Vec3( 0.f, 0.f, 0.f );
	float burn_amount = 1.f;	// 0 is nothing, 1 is fully gone

	uint step_count = 32;
	float depth = 0.01f;
	float mat_pad00[2];
};

// New format of material constants
struct dissolve_data_t
{
	Vec3 burn_start_color = Vec3( 1.f, 0.f, 0.f );
	float burn_edge_width = 0.1f;

	Vec3 burn_end_color = Vec3( 0.f, 0.f, 0.f );
	float burn_amount = 1.f;	// 0 is nothing, 1 is fully gone
};

struct fresnel_data_t
{
	Vec3 color = Vec3( 0.f, 1.f, 0.f );
	float fresnel_power = 1.f;
};

struct parallax_data_t
{
	uint step_count = 32;
	float depth = 0.01f;
	float mat_pad00[2];
};

struct image_effect_data_t
{
	float grayScale = 0.f;
	float effect_pad00[3];
};


struct tint_data_t
{
	float tint[4];
};

struct light_t
{
	Vec3 color = Vec3( 1.f, 1.f, 1.f );
	float intensity = 0.f;

	Vec3 position;
	float pad00;

	Vec3 direction = Vec3( 0.f, 0.f, -1.f );
	float direction_factor = 0.0f;

	Vec3 attenuation = Vec3( 0.f, 0.f, 1.f );
	float dot_inner_angle = -1.f;

	Vec3 spec_attenuation;
	float dot_outer_angle = -1.f;
};

struct light_constants_t
{
	Vec4 ambient = Vec4(1.f, 1.f, 1.f, 0.f);	// default: (1,1,1,0) - base level lighting in the scene (rgba, intensity)
	Vec4 emissive = Vec4( 1.f, 1.f, 1.f, 0.f );	// default: (1,1,1,0) - light this object emits (added to final equation)

	light_t lights[8];
	
	float diffuse_factor = 1.f;		// default: 1  - scales diffuse lighting in equation (lower values make an object absorb light
	float emissive_factor = 1.f;	// default: 1  - controls how much the emissive texture is added into the final equation

	float fog_near_distance = 0.f;
	float fog_far_distance = 75.f;

	Vec3 fog_near_color;
	float fog_pad00;
	Vec3 fog_far_color = Vec3(0.0784f, 0.0784f, 0.0784f); // 20/255
	float fog_pad01;
};


class RenderContext 
{
public:
	void StartUp( Window* window );
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void UpdateFrameTime( float deltaSeconds );
	void UpdateTintColor( Rgba8 tint );
	void UpdateImageEffect();

	void BindMaterialBuffer( RenderBuffer* ubo );

	void ClearScreen( const Rgba8& clearColor );
	void BeginCamera( const Camera& camera );
	void EndCamera ( const Camera& camera );

	void DrawVertexArray( int numVertexes, const Vertex_PCU* vertexes );
	void DrawVertexArray( const std::vector<Vertex_PCU>& verts );
	void Draw( int numVertexes, int vertexOffset = 0 );
	void DrawIndexed( int indexCount, int indexOffset = 0, int vertexOffset = 0 );
	void DrawMesh( GPUMesh* mesh );
	void DrawLine( const Vec2& start, const Vec2& end, const Rgba8& color, float thickness );
	void DrawRing( const Vec2& center, float radius, const Rgba8& color, float thickness );
	void DrawAABB2( const AABB2& bounds, const Rgba8& tint );
	void DrawQuad( const AABB2& bounds, float z, const Rgba8& tint );
	void DrawOBB2( const OBB2& box, const Rgba8& tint );
	void DrawDisc( const Vec2& center, float radius, const Rgba8& tint );
	void DrawCapsule( const Capsule2& capsule, const Rgba8& tint );
	void DrawPolygon( const std::vector<Vec2> points, const Rgba8& tint );

	void BindTexture( const Texture* constTex );
	void BindTexture( const Texture* constTex, const int slot);
	void BindNormalTexture( const Texture* constTex );
	void BindSpecularTexture( const Texture* constTex );
	void BindPatternTexture( const Texture* constTex );
	void BindSampler( Sampler* sampler );
	void BindSampler( Sampler* sampler, const int slot );
	void BindShader( Shader* shader );
	void BindShaderStateFromName( const char* shaderStateName );
	void BindShaderState( ShaderState* shaderState);
	void BindShaderFromFile( char const* filename );
	void BindMaterial( Material* material );

	void BindVertexBuffer( VertexBuffer* vbo );
	void BindIndexBuffer( IndexBuffer* ibo );
	void BindUniformBuffer( uint slot, RenderBuffer* ubo );
	void BindUniformBuffer( uint slot, const Camera* camera );

	// depth functions
	void EnableDepth( eCompareOp op, bool write );
	void DisableDepth();
	void ClearDepth( Texture* depthStencilTexture, float depth, uint stencil );

	Texture* CreateRenderTarget( IntVec2 texelSize );
	Texture* AcquireRenderTargetMatching( Texture* texture );
	void	 ReleaseRenderTarget( Texture* texture );
	Texture* GetBackBuffer() const;

	void CopyTexture( Texture* dst, Texture* src );
	//void ApplyEffect( Texture* dst, Texture* src, Material* mat );

	void StartEffecct( Texture* dst, Texture* src, Shader* shader );
	void EndEffect();

	Texture* GetFrameColorTarget(); // use to get swapchains backbuffer
	Texture* CreateOrGetTextureFromFile( const char* imageFilePath );
	BitmapFont* CreateOrGetBitmapFont( const char* bitmapFontFilePathNoExtension );
	Shader* GetOrCreateShader( const char* filename );
	ShaderState* GetShaderState( const char* filename );

	Vec3 ConvertRgba8ToVec3( Rgba8 rgba8 ) const;
	bool IsDrawing() { return m_isDrawing; }

public:
	void FinalizeState();
	bool CreateDepthStencilBuffer();

	BitmapFont* CreateBitmapFontFromFile( const char* FontFilePath );
	Texture* CreateTextureFromFile( const char* ImageFilePath );
	Texture* CreateTextureFromColor( Rgba8 color );


	void CreateRasterState();
	void CreateBlendState();
	void SetBlendMode( BlendMode blendMode );
	void SetCullMode( D3D11_CULL_MODE newCullMode );
	void SetFillMode( D3D11_FILL_MODE newFillMode );
	void SetFrontCounterClockwise( bool isCCW );
	void SetModelMatrix( Mat44 const& mat );

	// Lighting Methods
	void UpdateLight();
	void SetAmbientColor( Rgba8 color );
	void SetAmbientIntensity( float intensity );
	void SetAmbientLight( Rgba8 color, float intensity );

	void EnableLight( uint idx, light_t lightInfo );
	void DisableLight( uint idx );

	//------------------------------------------------------------------------------------------------
	// Fog
	//------------------------------------------------------------------------------------------------
	void EnableFog( float nearFog, float farFog, Rgba8 nearFogColor, Rgba8 farFogColor );
	void DisableFog();

	void SetShaderMode( eShaderMode mode );
	std::string GetShaderModeName( eShaderMode mode ) const;

	// point light
	void SetLightAttenuation( Vec3 att, int idx );
	Vec3 GetAttenuationFromAttMode( eAttenuationMode mode ) const;
	eAttenuationMode CovertAttToEnumAttMode( const Vec3 attenaution ) const;
	std::string GetAttenuationAsString( eAttenuationMode mode ) const;

	void SetSpecularFactor( const float specularFactor );
	void SetSpecularPower( const float specularPower );

	//void SetWorldCameraPosition( const Vec3& cameraPos );

private:
	std::vector< Texture* > m_textureVector;
	std::vector< Shader* > m_shaderVector;
	std::vector< BitmapFont* > m_loadedFonts;
	std::vector< ShaderState* > m_shaderStateList;

	std::vector<Texture*> m_renderTargetPool;
	int m_totalRenderTargetMade = 0;	// determine if leaking, debug purpose

	float thetaDegrees[64] = {};
	Vertex_PCU innerCircleVertexes[64] = {};

public:
	ID3D11Device*	m_device = nullptr;
	ID3D11Device1*	m_device1 = nullptr;
	ID3D11DeviceContext* m_context = nullptr; // immediate context
	ID3D11DeviceContext1* m_context1 = nullptr; 
	SwapChain* m_swapchain = nullptr;
	//----------------------------------------------------
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	//----------------------------------------------------
	Shader*			m_defaultShader		= nullptr;
	Shader*			m_currentShader		= nullptr;
	Texture*		m_frameColorTarget	= nullptr;

	RenderBuffer* m_frameUBO = nullptr;
	RenderBuffer* m_modelUBO = nullptr;
	RenderBuffer* m_tintUBO = nullptr;
	RenderBuffer* m_lightUBO = nullptr;
	RenderBuffer* m_materialUBO = nullptr;
	
	GPUMesh*		m_immediateMesh	= nullptr;
	VertexBuffer*	m_lastBoundVBO	= nullptr;
	IndexBuffer*	m_indicesBuffer = nullptr;

	ID3D11DepthStencilState* m_currentDepthStencilState = nullptr; 

	ID3D11Buffer* m_lastVBO = nullptr;
	bool m_isDrawing = false;

	Sampler* m_defaultSampler = nullptr;
	Texture* m_defaultColorTex = nullptr;
	Texture* m_defaultBackBuffer = nullptr;

	ID3D11BlendState* m_alphaBlendState;
	ID3D11BlendState* m_additiveBlendState;
	ID3D11BlendState* m_opaqueBlendState;

	ID3D11RasterizerState* m_rasterState = nullptr;

	buffer_attribute_t const* m_lastBoundLayout;
	buffer_attribute_t const* m_currentLayout;
	bool m_layoutHasChanged = false;
	bool m_shaderHasChanged = false;

	// light
	light_constants_t m_lightConstants;
	bool m_isLightDataDirty = true; // replace by enum eRenderContextDirtyBit

	//---------------------------------------------------------------------------------------------
	// model matrix data
	//---------------------------------------------------------------------------------------------
	float m_specularFactor = 1.f;
	float m_specularPower = 32.f;

	material_data_t m_materialData;
	image_effect_data_t m_imageEffectData;

	// image effect
	Camera* m_effectCamera = nullptr;

	// VR Related
	Vec2 m_recommendedSize = Vec2::ZERO;

	// MSAA
	UINT m_msaa = 4;
	UINT m_4xMsaaQuality = 1;
};
