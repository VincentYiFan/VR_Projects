#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/QuadDebugObject.hpp"
#include "Engine/Renderer/SphereDebugObject.hpp"
#include "Engine/Renderer/LineDebugObject.hpp"
#include "Engine/Renderer/ArrowDebugObject.hpp"
#include "Engine/Renderer/TextDebugObject.hpp"
#include "Engine/Renderer/BillboardTextDebugObject.hpp"
#include "Engine/Renderer/BasisDebugObject.hpp"
#include "Engine/Renderer/WireBoundDebugObject.hpp"
#include "Engine/Renderer/MeshDebugObject.hpp"
#include "Engine/Renderer/ScreenDebugObject_Point.hpp"
#include "Engine/Renderer/ScreenDebugObject_Line.hpp"
#include "Engine/Renderer/ScreenDebugObject_Quad.hpp"
#include "Engine/Renderer/ScreenDebugObject_Text.hpp"
#include "Engine/Renderer/ScreenDebugObject_Basis.hpp"
#include "Engine/Renderer/ConeDebugObject.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB3.hpp"


DebugRender* g_theDebugRenderSystem = nullptr; 
extern RenderContext* g_theRenderer; 

DebugRender::DebugRender()
{
	m_context = g_theRenderer;
}

void DebugRender::DebugRenderSystemStartup()
{
	m_clock = new Clock( Clock::GetMaster() );
}

void DebugRender::DebugRenderSystemShutdown()
{
	for( int i = 0; i < (int)m_debugObjects.size(); ++i )
	{
		if( m_debugObjects[i] != nullptr )
		{
			delete m_debugObjects[i];
			m_debugObjects[i] = nullptr;
		}
	}

	delete g_theDebugRenderSystem;
	g_theDebugRenderSystem = nullptr;
}

void DebugRender::EnableDebugRendering()
{
	m_isDebugRenderingActive = true;
}

void DebugRender::DisableDebugRendering()
{
	m_isDebugRenderingActive = false;
}

void DebugRender::ClearDebugRendering()
{
	for( int i = 0; i < (int)m_debugObjects.size(); ++i )
	{
		if( m_debugObjects[i] != nullptr )
		{
			delete m_debugObjects[i];
			m_debugObjects[i] = nullptr;
		}
	}

	m_debugObjects.clear();

	for( int i = 0; i < (int)m_screenDebugObjects.size(); ++i )
	{
		if( m_screenDebugObjects[i] != nullptr )
		{
			delete m_screenDebugObjects[i];
			m_screenDebugObjects[i] = nullptr;
		}
	}

	m_screenDebugObjects.clear();
}

void DebugRender::DebugRenderBeginFrame()
{
}

void DebugRender::DebugRenderWorldToCamera( Camera camera )
{
	m_camera = camera;

	// Check If Needed To Be Finalized
	if( m_context == nullptr )
	{
		m_context = camera.m_cameraUBO->m_owner;
	}

	uint tempClearMode = camera.m_clearMode;
	camera.SetClearMode( CLEAR_NONE );
	
	m_context->SetModelMatrix( Mat44::IDENTITY );

	for( int i = 0; i < (int)m_debugObjects.size(); ++i ) {
		if( m_debugObjects[i] != nullptr && !m_debugObjects[i]->m_timer->HasElapsed() ) {
			m_context->BindTexture( nullptr );
			g_theRenderer->SetModelMatrix( m_debugObjects[i]->m_transform.ToMatrix() );
			m_debugObjects[i]->DrawDebugObject();
		}
	}
	camera.SetClearMode( tempClearMode );
	m_context->UpdateTintColor( Rgba8::WHITE );
}

void DebugRender::DebugRenderScreenTo( Texture* texture )
{
	m_context = texture->GetRenderContext();

	Camera* camera = new Camera();
	camera->SetColorTarget( texture );
	Vec2 min = Vec2::ZERO;
	Vec2 max = texture->GetDimensions();

	m_screenCameraDimensions = AABB2( min, max );

	camera->SetOutputSize( max );
	camera->SetProjectionOrthographic( max.y, 10.f, -10.f );
	camera->SetOrthoView( Vec2::ZERO, Vec2( max.x, max.y ) );
	camera->IntialUBO( m_context );
	camera->m_canClearDepthBuffer = false;

	camera->SetClearMode( CLEAR_NONE );

	g_theDebugRenderSystem->m_context->BeginCamera( *camera );

	for( int i = 0; i < (int)m_screenDebugObjects.size(); ++i )
	{
		if( m_screenDebugObjects[i] != nullptr )
		{
			m_context->BindTexture( nullptr );
			m_screenDebugObjects[i]->DrawDebugObject();
		}
	}

	g_theDebugRenderSystem->m_context->EndCamera( *camera );
	camera->CleanUBO();

	delete camera;
	camera = nullptr;
}

void DebugRender::DebugRenderEndFrame()
{
	for( int i = 0; i < (int)m_debugObjects.size(); ++i )
	{
		if( m_debugObjects[i] )
		{
			if( m_debugObjects[i]->m_timer->HasElapsed() )
			{
				m_debugObjects[i]->MarkForGarbage();
			}


			if( m_debugObjects[i]->IsGarbage() )
			{
				delete m_debugObjects[i];
				m_debugObjects[i] = nullptr;
			}
		}	
	}

	for( int i = 0; i < (int)m_screenDebugObjects.size(); ++i )
	{
		if( m_screenDebugObjects[i] )
		{
			if( m_screenDebugObjects[i]->m_timer->HasElapsed() )
			{
				m_screenDebugObjects[i]->MarkForGarbage();
			}


			if( m_screenDebugObjects[i]->IsGarbage() )
			{
				delete m_screenDebugObjects[i];
				m_screenDebugObjects[i] = nullptr;
			}
		}
	}
}

void DebugRender::DebugAddWorldPoint( Vec3 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	SphereDebugObject* point = new SphereDebugObject( pos, size, start_color, end_color, duration, mode );
	point->m_isWireMode = false;
	m_debugObjects.push_back( point );
}

void DebugRender::DebugAddWorldPoint( Vec3 pos, float size, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, size, color, color, duration, mode );
}

void DebugRender::DebugAddWorldPoint( Vec3 pos, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, 0.5f, color, duration, mode );
}

void DebugRender::DebugAddWorldLine( Vec3 p0, Rgba8 p0_color, Vec3 p1, Rgba8 p1_color, float thickness, float duration, bool isWire, eDebugRenderMode mode )
{
	UNUSED( mode );
	LineDebugObject* line = new LineDebugObject( p0, p0_color, p1, p1_color, thickness, duration, isWire );
	m_debugObjects.push_back( line );
}

void DebugRender::DebugAddWorldLine( Vec3 start, Vec3 end, Rgba8 color, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );
	DebugAddWorldLine( start, color, end, color, 0.5f, duration, mode );
}

void DebugRender::DebugAddWorldQuad( Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, AABB2 uvs, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );
	QuadDebugObject* quad = new QuadDebugObject( p0, p1, p2, p3, uvs, start_color, end_color, duration );
	m_debugObjects.push_back( quad );
}

void DebugRender::DebugAddWorldArrow( Vec3 start, Rgba8 startColor, Vec3 end, Rgba8 endColor, float duration, float thickness, eDebugRenderMode mode )
{
	UNUSED( mode );
	ArrowDebugObject* arrow = new ArrowDebugObject( start, startColor, end, endColor, thickness, duration );
	m_debugObjects.push_back( arrow );
}

void DebugRender::DebugAddWorldArrow( Vec3 start, Vec3 end, Rgba8 color, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );
	DebugAddWorldArrow( start, color, Vec3::ZERO, color, duration, 1.f, mode );
}

void DebugRender::DebugAddWorldText( Mat44 basis, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, char const* text, eDebugRenderMode mode )
{
	UNUSED( mode );
	TextDebugObject* worldText = new TextDebugObject( basis, pivot, start_color, end_color, duration, text );
	m_debugObjects.push_back( worldText );
}

void DebugRender::DebugAddWorldText( Mat44 basis, Vec2 pivot, Rgba8 color, float duration, float fontSize, char const* text, eDebugRenderMode mode )
{
	UNUSED( mode );
	TextDebugObject* worldText = new TextDebugObject( basis, pivot, color, fontSize, text, duration );
	InsertOrPushBackIntoVector( m_debugObjects, worldText );
}

void DebugRender::DebugAddWorldBillboardText( Vec3 origin, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, char const* text, eDebugRenderMode mode )
{
	UNUSED( mode );
	BillboardTextDebugObject* BBText = new BillboardTextDebugObject( origin, pivot, start_color, end_color, duration, text );
	m_debugObjects.push_back( BBText );
}

void DebugRender::DebugAddWorldBasis( Mat44 basis, Rgba8 start_tint, Rgba8 end_tint, float thickness, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );
	BasisDebugObject* basisObject = new BasisDebugObject( basis, start_tint, end_tint, thickness, duration );
	m_debugObjects.push_back( basisObject );
}

void DebugRender::DebugAddWorldBasis( Mat44 basis, float thickness, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );
	DebugAddWorldBasis( basis, Rgba8::WHITE, Rgba8::WHITE, thickness, duration, mode );
}

void DebugRender::DebugAddWorldWireSphere( Vec3 pos, float radius, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );
	SphereDebugObject* point = new SphereDebugObject( pos, radius, start_color, end_color, duration, true );
	m_debugObjects.push_back( point );
}

void DebugRender::DebugAddWorldWireSphere( Vec3 pos, float radius, Rgba8 color, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );
	DebugAddWorldWireSphere( pos, radius, color, color, duration, mode );
}

void DebugRender::DebugAddWorldWireBounds( OBB3 bounds, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );
	WireBoundDebugObject* wireBound = new WireBoundDebugObject( bounds, start_color, end_color, duration );
	m_debugObjects.push_back( wireBound );
}

void DebugRender::DebugAddWorldWireBounds( OBB3 bounds, Rgba8 color, float duration, bool isWire, eDebugRenderMode mode )
{
	UNUSED( mode );
	WireBoundDebugObject* wireBound = new WireBoundDebugObject( bounds, color, color, duration, isWire );
	m_debugObjects.push_back( wireBound );
}

void DebugRender::DebugAddWorldWireBounds( AABB3 bounds, Rgba8 color, float duration, bool isWire, eDebugRenderMode mode)
{
	UNUSED( mode );
	WireBoundDebugObject* wireBound = new WireBoundDebugObject( bounds, color, color, duration, isWire );
	InsertOrPushBackIntoVector( m_debugObjects, wireBound );
}

void DebugRender::DebugAddWorldCone( Cone cone, Rgba8 startColor, Rgba8 endColor, float duration )
{
	ConeDebugObject* coneDebugObject = new ConeDebugObject( cone, startColor, endColor, duration );
	InsertOrPushBackIntoVector( m_debugObjects, coneDebugObject );
}

void DebugRender::DebugAddScreenPoint( Vec2 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration )
{
	ScreenDebugObject_Point* point = new ScreenDebugObject_Point( pos, size, start_color, end_color, duration );
	m_screenDebugObjects.push_back( point );
}

void DebugRender::DebugAddScreenPoint( Vec2 pos, float size, Rgba8 color, float duration )
{
	DebugAddScreenPoint( pos, size, color, color, duration );
}

void DebugRender::DebugAddScreenPoint( Vec2 pos, Rgba8 color )
{
	DebugAddScreenPoint( pos, 30.f, color, color, 999999.f );
}

void DebugRender::DebugAddScreenLine( Vec2 p0, Rgba8 p0_color, Vec2 p1, Rgba8 p1_color, float duration )
{
	ScreenDebugObject_Line* line = new ScreenDebugObject_Line( p0, p0_color, p1, p1_color, duration );
	m_screenDebugObjects.push_back( line );
}

void DebugRender::DebugAddScreenLine( Vec2 p0, Vec2 p1, Rgba8 color, float duration )
{
	DebugAddScreenLine( p0, color, p1, color, duration );
}

void DebugRender::DebugAddScreenQuad( AABB2 bounds, Rgba8 start_color, Rgba8 end_color, float duration )
{
	ScreenDebugObject_Quad* quad = new ScreenDebugObject_Quad( bounds, start_color, end_color, duration );
	m_screenDebugObjects.push_back( quad );
}

void DebugRender::DebugAddScreenQuad( AABB2 bounds, Rgba8 color, float duration )
{
	DebugAddScreenQuad( bounds, color, color, duration );
}


void DebugRender::DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, AABB2 uvs, Rgba8 tint, float duration )
{
	ScreenDebugObject_Quad* quad = new ScreenDebugObject_Quad( bounds, tint, tex, uvs, duration );
	InsertOrPushBackIntoVector( m_screenDebugObjects , quad );
	//m_screenDebugObjects.push_back( quad );
}

void DebugRender::DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, Rgba8 tint, float duration )
{
	DebugAddScreenTexturedQuad( bounds, tex, AABB2::ZERO_TO_ONE, tint, duration );
}

void DebugRender::DebugAddScreenText( Vec4 pos, Vec2 pivot, float textSize, Rgba8 start_color, Rgba8 end_color, float duration, char const* text )
{
	ScreenDebugObject_Text* theText = new ScreenDebugObject_Text( pos, pivot, textSize, start_color, end_color, duration, text );
	InsertOrPushBackIntoVector( m_screenDebugObjects, theText );
}

void DebugRender::DebugAddScreenBasis( Vec2 screen_origin_location, Mat44 basis_to_render, Rgba8 start_tint, Rgba8 end_tint, float duration )
{
	ScreenDebugObject_Basis* basis = new ScreenDebugObject_Basis( screen_origin_location, basis_to_render, start_tint, end_tint, duration );
	m_screenDebugObjects.push_back( basis );
}

void DebugRender::DebugAddScreenBasis( Vec2 screen_origin_location, Mat44 basis_to_render, Rgba8 tint, float duration )
{
	DebugAddScreenBasis( screen_origin_location, basis_to_render, tint, tint, duration );
}

void DebugRender::InsertOrPushBackIntoVector( std::vector<DebugRenderObject*>& vector, DebugRenderObject* object )
{
	bool foundEmptySlot = false;
	// avoid infinite size of vector
	for( int i = 0; i < (int)vector.size(); ++i ) {
		if( vector[i] == nullptr ) {
			vector[i] = object;
			foundEmptySlot = true;
			break;
		}
	}
	if( foundEmptySlot == false ) {
		vector.push_back( object );
	}
}

