#pragma once
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/Cone.hpp"
#include <vector>

//----------------------------------------------------------------------------------------------------------------------------
class Clock;
//----------------------------------------------------------------------------------------------------------------------------

class DebugRender
{
	/************************************************************************/
	/*                                                                      */
	/* TYPES                                                                */
	/*                                                                      */
	/************************************************************************/

	// Only effects "world" mode stuff; 


	/************************************************************************/
	/*                                                                      */
	/* FUNCTION PROTOTYPES                                                  */
	/*                                                                      */
	/************************************************************************/
	//------------------------------------------------------------------------
	//  System
	//------------------------------------------------------------------------

public:
	DebugRender();

	// setup
	void DebugRenderSystemStartup();					// may be used to allocate resources to the system
	void DebugRenderSystemShutdown();					// cleans up the system to prevent leaks.

	// control
	void EnableDebugRendering();					
	void DisableDebugRendering();
	void ClearDebugRendering(); 

	// output
	void DebugRenderBeginFrame();						// Does nothing, here for completeness.
	void DebugRenderWorldToCamera( Camera camera );	// Draws all world objects to this camera 
	void DebugRenderScreenTo( Texture* texture );		// Draws all screen objects onto this texture (screen coordinate system is up to you.  I like a 1080p default)
	void DebugRenderEndFrame();							// Clean up dead objects

	//------------------------------------------------------------------------
	// World Rendering
	//------------------------------------------------------------------------

	// points
	void DebugAddWorldPoint( Vec3 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
	void DebugAddWorldPoint( Vec3 pos, float size, Rgba8 color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
	void DebugAddWorldPoint( Vec3 pos, Rgba8 color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

	// lines
	void DebugAddWorldLine( Vec3 p0, Rgba8 p0_color, Vec3 p1, Rgba8 p1_color, float thickness, float duration = 0.f, bool isWire = false, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
	void DebugAddWorldLine( Vec3 start, Vec3 end, Rgba8 color, float duration = 0.f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );


	// quads
	void DebugAddWorldQuad( Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, AABB2 uvs, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

	// arrows
	void DebugAddWorldArrow( Vec3 start,Rgba8 startColor, Vec3 end, Rgba8 endColor, float duration,float thickness, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH ); 
	void DebugAddWorldArrow( Vec3 start, Vec3 end, Rgba8 color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH ); 


	// text
	void DebugAddWorldText( Mat44 basis, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, char const* text, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH ); 
	void DebugAddWorldText( Mat44 basis, Vec2 pivot, Rgba8 color, float duration, float fontSize, char const* text, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH ); 

	// billboard text
	void DebugAddWorldBillboardText( Vec3 origin, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, char const* text, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH  ); 


	// basis
	void DebugAddWorldBasis( Mat44 basis, Rgba8 start_tint, Rgba8 end_tint, float thickness, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
	void DebugAddWorldBasis( Mat44 basis, float thickness, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

	// wire sphere
	void DebugAddWorldWireSphere( Vec3 pos, float radius, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
	void DebugAddWorldWireSphere( Vec3 pos, float radius, Rgba8 color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

	// wire cube
	void DebugAddWorldWireBounds( OBB3 bounds, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
	void DebugAddWorldWireBounds( OBB3 bounds, Rgba8 color, float duration = 0.0f, bool isWire = true, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
	void DebugAddWorldWireBounds( AABB3 bounds, Rgba8 color, float duration = 0.0f, bool isWire = true, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

	// cone
	void DebugAddWorldCone( Cone cone, Rgba8 startColor, Rgba8 endColor, float duration = 0.f );

	// render mesh
	//void DebugAddWireMeshToWorld( Mat44 model, GPUMesh* mesh, Rgba8 start_tint, Rgba8 end_tint, float duration, eDebugRenderMode mode );

	//------------------------------------------------------------------------
	// Screen Rendering
	//------------------------------------------------------------------------

	// points
	void DebugAddScreenPoint( Vec2 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration );
	void DebugAddScreenPoint( Vec2 pos, float size, Rgba8 color, float duration );
	void DebugAddScreenPoint( Vec2 pos, Rgba8 color ); // assumed size;


	// lines
	void DebugAddScreenLine( Vec2 p0, Rgba8 p0_color, Vec2 p1, Rgba8 p1_color, float duration );
	void DebugAddScreenLine( Vec2 p0, Vec2 p1, Rgba8 color, float duration = 0.f );


	// quad
	void DebugAddScreenQuad( AABB2 bounds, Rgba8 start_color, Rgba8 end_color, float duration );
	void DebugAddScreenQuad( AABB2 bounds, Rgba8 color, float duration = 0.0f );


	// textured quad
	void DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, AABB2 uvs, Rgba8 tint, float duration = 0.0f );
	void DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, Rgba8 tint = Rgba8::WHITE, float duration = 0.0f ); // assume UVs are full texture


	// text
	void DebugAddScreenText( Vec4 pos, Vec2 pivot, float textSize, Rgba8 start_color, Rgba8 end_color, float duration, char const* text ); 


	// screen basis
	void DebugAddScreenBasis( Vec2 screen_origin_location, Mat44 basis_to_render, Rgba8 start_tint, Rgba8 end_tint, float duration );
	void DebugAddScreenBasis( Vec2 screen_origin_location, Mat44 basis_to_render, Rgba8 tint = Rgba8::WHITE, float duration = 0.0f );

	// helper functions
	void InsertOrPushBackIntoVector( std::vector<DebugRenderObject*>& vector, DebugRenderObject* object );

public:
	RenderContext* m_context = nullptr;

	std::vector<DebugRenderObject*> m_debugObjects;
	std::vector<DebugRenderObject*> m_screenDebugObjects;

	Clock* m_clock = nullptr;
	Camera m_camera; // Delete later

	AABB2 m_screenCameraDimensions = AABB2::ZERO_TO_ONE;

	bool m_isDebugRenderingActive = false;
};

extern DebugRender* g_theDebugRenderSystem; 
