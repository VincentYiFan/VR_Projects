#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

eBillboardMode GetBillboardModeFromName( std::string const& modeName )
{
	eBillboardMode mode = eBillboardMode::BILLBOARD_MODE_CAMERA_FACING_XY;
	if( modeName == "CameraFacingXY" )
	{
		mode = eBillboardMode::BILLBOARD_MODE_CAMERA_FACING_XY;
	}
	else if( modeName == "CameraFacingXYZ" )
	{
		mode = eBillboardMode::BILLBOARD_MODE_CAMERA_FACING_XYZ;
	}
	else if( modeName == "CameraOpposingXY" )
	{
		mode = eBillboardMode::BILLBOARD_MODE_CAMERA_OPPOSING_XY;
	}
	else if( modeName == "CameraOpposingXYZ" )
	{
		mode = eBillboardMode::BILLBOARD_MODE_CAMERA_OPPOSING_XYZ;
	}
	else
	{
		// ERROR
	}

	return mode;
}


void GetBillboardQuad( Vec3& out_BL, Vec3& out_BR, Vec3& out_TR, Vec3& out_TL, Camera const& cam, Vec3 center, Vec2 size, eBillboardMode mode )
{
	Vec3 forward;
	Vec3 left;
	Vec3 up = Vec3( 0.f, 0.f, 1.f );

	switch( mode )
	{
	case eBillboardMode::BILLBOARD_MODE_CAMERA_FACING_XY:
	{
		Vec3 dispToCam = cam.m_transform.m_position - center;
		forward = Vec3( dispToCam.x, dispToCam.y, 0.f ).GetNormalized();
		left = Vec3( -forward.y, forward.x, 0.f );
	}	break;


	case eBillboardMode::BILLBOARD_MODE_CAMERA_OPPOSING_XY:
	{
		forward = -cam.GetForwardVectorXY();
		left = Vec3( -forward.y, forward.x, 0.f );
	}	break;
		
	case eBillboardMode::BILLBOARD_MODE_CAMERA_FACING_XYZ:
	{
		forward = ( cam.m_transform.m_position - center ).GetNormalized();
		Vec3 forwardXY = Vec3( forward.x, forward.y, 0.f ).GetNormalized();
		left = Vec3( -forwardXY.y, forwardXY.x, 0.f );

		up = CrossProduct( forward, left );
	}	break;


	case eBillboardMode::BILLBOARD_MODE_CAMERA_OPPOSING_XYZ:
	{
		forward = -cam.GetForwardVector();

		left = CrossProduct( Vec3( 0.f, 0.f,1.f), forward ).GetNormalized();

		up = CrossProduct( forward, left ).GetNormalized();

		//Vec3 cameraLeft = Vec3( 0.f, 1.f, 0.f );
		//cameraLeft = cam.m_transform.ToMatrix().TransformVector3D( cameraLeft );
		//left = -cameraLeft.GetNormalized();
		//
		//Vec3 cameraUp = Vec3( 0.f, 0.f ,1.f );
		//cameraUp = cam.m_transform.ToMatrix().TransformVector3D( cameraUp );
		//up = cameraUp.GetNormalized();
	}	break;

	default:	g_theConsole->Error( "ERROR:Failed to GetBillboard Quad: Invalid billboard mode" );
	}


	out_BL = center + ( 0.5f * size.x * left ) - ( 0.5f * size.y * up );
	out_BR = center - ( 0.5f * size.x * left ) - ( 0.5f * size.y * up );
	out_TL = center + ( 0.5f * size.x * left ) + ( 0.5f * size.y * up );
	out_TR = center - ( 0.5f * size.x * left ) + ( 0.5f * size.y * up );
}

void AppendVertsForAABB2D( std::vector<Vertex_PCU>& verts, const AABB2& localBounds, const Rgba8 color )
{
	Vec2 minVec2 = localBounds.mins;
	Vec2 maxVec2 = localBounds.maxs;

	verts.push_back( Vertex_PCU( Vec3( maxVec2.x, minVec2.y, 0.f ), color, Vec2( 1.f, 0.f ) ) );
	verts.push_back( Vertex_PCU( Vec3( minVec2.x, maxVec2.y, 0.f ), color, Vec2( 0.f, 1.f ) ) );
	verts.push_back( Vertex_PCU( Vec3( minVec2.x, minVec2.y, 0.f ), color, Vec2( 0.f, 0.f ) ) );
														
	verts.push_back( Vertex_PCU( Vec3( maxVec2.x, minVec2.y, 0.f ), color, Vec2( 1.f, 0.f ) ) );
	verts.push_back( Vertex_PCU( Vec3( maxVec2.x, maxVec2.y, 0.f ), color, Vec2( 1.f, 1.f ) ) );
	verts.push_back( Vertex_PCU( Vec3( minVec2.x, maxVec2.y, 0.f ), color, Vec2( 0.f, 1.f ) ) );
}


void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, const AABB2& localBounds, const Rgba8 color, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec2 minVec2 = localBounds.mins;
	Vec2 maxVec2 = localBounds.maxs;

	verts.push_back( Vertex_PCU( Vec3( maxVec2.x, minVec2.y, 0.f ), color, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( minVec2.x, maxVec2.y, 0.f ), color, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( minVec2.x, minVec2.y, 0.f ), color, Vec2( uvAtMins.x, uvAtMins.y ) ) );

	verts.push_back( Vertex_PCU( Vec3( maxVec2.x, minVec2.y, 0.f ), color, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( maxVec2.x, maxVec2.y, 0.f ), color, Vec2( uvAtMaxs.x, uvAtMaxs.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( minVec2.x, maxVec2.y, 0.f ), color, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
}

void AppendVertsForOBB2D( std::vector<Vertex_PCU>& verts, const OBB2& localBounds, const Rgba8 color )
{
	UNUSED( verts );
	UNUSED( localBounds );
	UNUSED( color );
	// #SD1ToDo
}
