#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Renderer/Transform.hpp"
#include <vector>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class Texture;
class RenderBuffer;
class RenderContext;
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef unsigned int uint;

enum eCameraClearBit : unsigned int
{
	CLEAR_NONE = 0,

	CLEAR_COLOR_BIT	  = (1 << 0), 
	CLEAR_DEPTH_BIT   = (1 << 1), 
	CLEAR_STENCIL_BIT = (1 << 2),

	CLEAR_ALL = ~0U,
};
//typedef uint eCameraClearBit;

enum class eWorldBasisConvention
{
	WORLD_BASIS_XFORWARD_YLEFT_ZUP,
	WORLD_BASIS_XRIGHT_YUP_ZBACK,
	NUM_WORLD_BASES
};

struct camera_data_t
{
	Mat44 projection;
	Mat44 view;
	Mat44 world_to_clip;	// view * projection 
	Mat44 clip_to_world;	// inverse(view * projection)
	Mat44 camera_model;

	Vec3 camera_position;	float cam_pad00;
	Vec3 camera_right;		float cam_pad01;
	Vec3 camera_up;			float cam_pad02;
	Vec3 camera_forward;	float cam_pad03;
};

class Camera 
{
public:
	~Camera();

	void		SetOutputSize( Vec2 size ); 

	Vec2		ClientToWorldPosition( Vec2 clientPos );
	float		GetAspectRatio() const { return m_aspectRatio; }
	float		GetOutputAspectRatio();

	void		SetDepthStencilTarget( Texture* texture );
	void		SetClearMode( unsigned int clearFlags, Rgba8 color = Rgba8::BLACK, float depth = 0.0f, unsigned int stencil = 0 );

	void		ClearDepth( Texture* depthStencilTexture, float depth, uint stencil );

	void		SetPosition( const Vec3& position );
	void		Translate( const Vec3& translation );
	void		SetPitchRollYawRotation( float pitch, float roll, float yaw );

	void		SetProjectionOrthographic( float height, float nearZ, float farZ );
	void		SetProjectionPerspective( float fovDegrees, float nearZClip, float farZClip );
	void		SetDepthStencilBuffer( Texture* texture );
	void		SetCanClearDepthBuffer( bool canClearDepth );

	void		SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );
	Vec2		GetOrthoBottonLeft() const { return m_orthoButtomLeft; }
	Vec2		GetOrthoTopRight() const { return m_orthoTopRight; }


	// Setters
	void		SetProjectionMatrix( Mat44 projectionMatrix );
	
	// Accessors
	Mat44		GetViewMatrix() const;          
	Mat44		GetProjectionMatrix() const;
	

	// Helpers
	Vec3		ClientToWorld( Vec2 client, float ndcZ );
	Vec3		WorldToClient( Vec3 worldPos );

	void		UpdateUBO();
	void		IntialUBO( RenderContext* ctx );
	void		CleanUBO();

	void		SetColorTarget( Texture* texture );
	void		SetColorTarget( int index, Texture* target );

	int			GetColorTargetCount() const;
	Texture*	GetColorTarget() const;
	Texture*	GetColorTarget( int index ) const;
	Rgba8		GetClearColor() const { return m_clearColor; }
	uint		GetClearStencil() const { return m_clearStencil; }
	Vec3		GetOrthoMin();
	
	// Camera forward vector
	Vec3		GetForwardVector() const;
	Vec3		GetForwardVectorXY() const;
	Vec2		GetForwardVector2D() const;

	
public:
	Transform m_transform;

	Vec2 m_orthoMin;	  // For Orthographic
	Vec2 m_orthoMax;	  // For Orthographic

	Vec2 m_outputSize;
	Vec3 m_position; // Is already inside m_transform

	Vec2 m_orthoButtomLeft = Vec2::ZERO;
	Vec2 m_orthoTopRight   = Vec2::ZERO;

	RenderBuffer* m_cameraUBO = nullptr;

	uint	 m_clearMode	= 0;
	Rgba8	 m_clearColor	= Rgba8::BLACK;
	float	 m_clearDepth	= 1.0f;
	uint	 m_clearStencil	= 0; 
	
	float m_aspectRatio = 0.f;
	float m_nearZ = 0.f;
	float m_farZ  = 0.f;
	Mat44 m_projection = Mat44::IDENTITY;

	bool m_canClearDepthBuffer = false;

	Texture* m_depthStencilTarget = nullptr;

	std::vector<Texture*> m_colorTargets;

	// Camera's Basis Convention
	eWorldBasisConvention m_convention = eWorldBasisConvention::WORLD_BASIS_XRIGHT_YUP_ZBACK;
};