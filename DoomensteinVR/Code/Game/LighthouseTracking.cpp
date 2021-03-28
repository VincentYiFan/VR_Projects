#include "Game/LighthouseTracking.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Physics/GameObject.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "stdafx.h"
#include <stdarg.h>
#include <windows.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#pragma warning(disable: 26812)
#pragma comment (lib, "openvr_api.lib")

//-------------------------------------------------------------------------------------------------------------
namespace Memory
{
template <class T> void SafeDelete( T& t )
{
	if( t )
	{
		delete t;
		t = nullptr;
	}
}

template <class T> void SafeDeleteArr( T& t )
{
	if( t )
	{
		delete[]t;
		t = nullptr;
	}
}

template <class T> void SafeRelease( T& t )
{
	if( t )
	{
		t->Release();
		t = nullptr;
	}
}
}

//-------------------------------------------------------------------------------------------------------------
LighthouseTracking::~LighthouseTracking()
{	
}

//-------------------------------------------------------------------------------------------------------------
LighthouseTracking::LighthouseTracking()
	: m_nRenderWidth(0)
	, m_nRenderHeight(0)
{
}

//-------------------------------------------------------------------------------------------------------------
void LighthouseTracking::StartUp( Window* window )
{
	m_theWindow = window;

	if( !BInit() )
	{
		g_theConsole->Error( "Failed to Created Lighthouse!" );
	}
	else 
	{
		g_theConsole->Printf( "Lighthouse Tracking Successfully Created" );
	}
}

//-------------------------------------------------------------------------------------------------------------
void LighthouseTracking::ShutDown()
{
	delete m_controllerGameObject;
	m_controllerGameObject = nullptr;

	if( m_pHMD )
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

	for( std::vector< GameObject* >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
	{
		delete( *i );
	}
	m_vecRenderModels.clear();
}

//-------------------------------------------------------------------------------------------------------------
bool LighthouseTracking::BInit()
{
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

	if( eError != vr::VRInitError_None )
	{
		m_pHMD = NULL;
		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		g_theConsole->Error( buf );
		//ERROR_AND_DIE( buf );
		return false;
	}

	if( m_pHMD )
	{
		m_pHMD->GetRecommendedRenderTargetSize( &m_nRenderWidth, &m_nRenderHeight );
		g_theConsole->Printf( "Recommended RenderTarget Size: width = %d, height = %d", m_nRenderWidth, m_nRenderHeight );
		g_gameConfigBlackboard.SetValue( "RecommendedRenderTargetSize", Stringf( "%d,%d", m_nRenderWidth * 2, m_nRenderHeight ) );
	}

	m_pRenderModels = ( vr::IVRRenderModels* )vr::VR_GetGenericInterface( vr::IVRRenderModels_Version, &eError );
	if( !m_pRenderModels )
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		g_theConsole->Error( buf );
		return false;
	}

	// Ensure VR Compositor is available, otherwise getting poses causes a crash
	if( !BInitCompositor() )
	{
		g_theConsole->Error( "Failed to initialized VR Compositor!" );
		return false;
	}
	else
	{
		g_theConsole->Printf( "Successfully initialized VR Compositor" );
	}


	m_fScale = 0.3f;
	m_fScaleSpacing = 4.0f;

	m_fNearClip = 0.1f;
	m_fFarClip = 30.0f;


	SetupControllerPulseTimers();
	SetupHideControllerModelTimer();
	SetupCameras();
	SetupRenderModels();

	return true;
}

//-------------------------------------------------------------------------------------------------------------
bool LighthouseTracking::BInitCompositor()
{
	//vr::EVRInitError peError = vr::VRInitError_None;

	if( !vr::VRCompositor() )
	{
		g_theConsole->Error( "Compositor initialization failed. See log file for details" );
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------------------
void LighthouseTracking::RunMainLoop()
{
	bool bQuit = false;
	if( !bQuit )
	{
		bQuit = HandleInput();

		if( !m_controllerPulseTimers[0].HasElapsed() )
		{
			vr::TrackedDeviceIndex_t controllerDeviceIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_LeftHand );
			unsigned short usDurationMicroSec = static_cast<unsigned short>(m_controllerPulseTimers[0].GetSecondsRemaining() * 1000000);
			m_pHMD->TriggerHapticPulse( controllerDeviceIndex, 0, usDurationMicroSec );
		}
		if( !m_controllerPulseTimers[1].HasElapsed() )
		{
			vr::TrackedDeviceIndex_t controllerDeviceIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_RightHand );
			unsigned short usDurationMicroSec = static_cast<unsigned short>(m_controllerPulseTimers[1].GetSecondsRemaining() * 1000000);
			m_pHMD->TriggerHapticPulse( controllerDeviceIndex, 0, usDurationMicroSec );
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
bool LighthouseTracking::HandleInput()
{
	if( !m_pHMD ) { // m_pHMD is Invalid
		return true;
	}

	// Process SteamVR events
	vr::VREvent_t event;
	if( m_pHMD->PollNextEvent( &event, sizeof( event ) ) )
	{
		ProcessVREvent( event );
	}

	//PrintPositionalData();

	// Process SteamVR controller state
	//for( vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++ )
	//{
	//	vr::VRControllerState_t state;
	//	if( m_pHMD->GetControllerState( unDevice, &state, sizeof( state ) ) )
	//	{
	//		m_rbShowTrackedDevice[unDevice] = state.ulButtonPressed == 0;
	//	}
	//}

	return false;
}

//-------------------------------------------------------------------------------------------------------------
void LighthouseTracking::ProcessVREvent( vr::VREvent_t const& event )
{
	switch( event.eventType )
	{
	case vr::VREvent_TrackedDeviceActivated:
		{
			SetupRenderModelForTrackedDevice( event.trackedDeviceIndex );
			dprintf( "Device %u attached. Setting up render model.\n", event.trackedDeviceIndex );
		}
		break;
	case vr::VREvent_TrackedDeviceDeactivated:
		{
			dprintf( "Device %u detached.\n", event.trackedDeviceIndex );
		}
		break;
	case vr::VREvent_TrackedDeviceUpdated:
		{
			dprintf( "Device %u updated.\n", event.trackedDeviceIndex );
		}
		break;
		

	default: 
		{
			if( event.eventType >= 200 && event.eventType <= 203 ) //Button events range from 200-203
			{
				DealWithButtonEvent( event );
			}
		}
	}
}

void LighthouseTracking::DealWithButtonEvent( vr::VREvent_t const& event )
{
	vr::VRControllerState_t controllerState;
	m_pHMD->GetControllerState( event.trackedDeviceIndex, &controllerState, sizeof(vr::VRControllerState_t) );


	vr::ETrackedDeviceClass trackedDeviceClass = m_pHMD->GetTrackedDeviceClass( event.trackedDeviceIndex );
	if( trackedDeviceClass != vr::ETrackedDeviceClass::TrackedDeviceClass_Controller ) {
		return;
	}

	vr::ETrackedControllerRole role = m_pHMD->GetControllerRoleForTrackedDeviceIndex( event.trackedDeviceIndex );

	std::string roleStr;
	if( role == vr::TrackedControllerRole_Invalid ) {
		// The controller is probably not visible to a base station.
	}
	else if( role == vr::TrackedControllerRole_LeftHand )
	{
		roleStr = "LeftHand";
	}
	else if( role == vr::TrackedControllerRole_RightHand )
	{
		roleStr = "RightHand";
	}

	switch( event.data.controller.button )
	{
	case vr::k_EButton_SteamVR_Trigger:
		{
			switch( event.eventType )
			{
			case vr::VREvent_ButtonPress:
				{
					//g_theConsole->Printf( "Trigger Pressed!" );
					std::string str = Stringf( "FIRE role=%s", roleStr.c_str() );
					g_theEventSystem->FireEventWithValue( str );

					// Add controller vibrate
					AddHapticPulse( role, CONTROLLER_PULSE_DURATION );
				}
				break;
			case vr::VREvent_ButtonUnpress:
				{
					//g_theConsole->Printf( "Trigger UnPressed!" );
				}
				break;
			}
		}
		break;

	case vr::k_EButton_SteamVR_Touchpad:
		{
			switch( event.eventType ) {
			case vr::VREvent_ButtonPress:
			{
				// Center
				FloatRange teleportRangeX = FloatRange( -0.6f, 0.6f );
				FloatRange teleportRangeY = FloatRange( -0.6f, 0.6f );
				if( teleportRangeX.IsInRange( controllerState.rAxis[0].x ) && teleportRangeY.IsInRange( controllerState.rAxis[0].y ) )
				{
					std::string str = Stringf( "BEGIN_TELEPORT role=%s", roleStr.c_str() );
					g_theEventSystem->FireEventWithValue( str );
				}

				// Left
				if( controllerState.rAxis[0].x < -0.61f )
				{
					if( g_theGame->m_locomoiton == eLocomotion::TELEPORTATION )
					{
						m_hideControllerModelTimer.SetSeconds( 0.03f ); // Avoid model flickering issue
						g_theGame->GetPlayer()->m_yawDegrees += 45.f;
						g_theGame->m_worldCameraLeft.m_transform.m_orientationYaw = g_theGame->GetPlayer()->m_yawDegrees;
						g_theGame->m_worldCameraRight.m_transform.m_orientationYaw = g_theGame->GetPlayer()->m_yawDegrees;
						m_previousHMDYawDegrees = ConvertRadiansToDegrees( GetHMDEulerAngles().x );
					}

				}
				// Right
				else if( controllerState.rAxis[0].x > 0.61f )
				{
					if( g_theGame->m_locomoiton == eLocomotion::TELEPORTATION )
					{
						m_hideControllerModelTimer.SetSeconds( 0.03f );
						g_theGame->GetPlayer()->m_yawDegrees -= 45.f;
						g_theGame->m_worldCameraLeft.m_transform.m_orientationYaw = g_theGame->GetPlayer()->m_yawDegrees;
						g_theGame->m_worldCameraRight.m_transform.m_orientationYaw = g_theGame->GetPlayer()->m_yawDegrees;
						m_previousHMDYawDegrees = ConvertRadiansToDegrees( GetHMDEulerAngles().x );
					}
				}

				FloatRange movementRangeX = FloatRange( -0.5f, 0.5f );
				if( movementRangeX.IsInRange( controllerState.rAxis[0].x ) )
				{
					if( controllerState.rAxis[0].y > 0.61f )		// top
					{
						g_theGame->PlayerRequestMoveForward();
					}
					else if( controllerState.rAxis[0].y < -0.61f )	// bottom
					{
						g_theGame->PlayerRequestMoveBackward();
					}
				}
			} 
			break;

			/*case vr::VREvent_ButtonTouch:
			{
				if( controllerState.rAxis[0].y > 0.61f ) 
				{
					g_theConsole->Printf( "Top" );
				}
				else if( controllerState.rAxis[0].y < -0.61f )
				{
					g_theConsole->Printf( "Down" );
				}
			}
			break;*/

			case vr::VREvent_ButtonUnpress:
			{
				std::string str = Stringf( "END_TELEPORT role=%s", roleStr.c_str() );
				g_theEventSystem->FireEventWithValue( str );

				g_theGame->m_isTouchpadTopButtonDown = false;
				g_theGame->m_isTouchpadBottomButtonDown = false;
			}
			break;
			}
		}
		break;
	}

}

//-------------------------------------------------------------------------------------------------------------
void LighthouseTracking::RenderFrame()
{
	if( !IsValid() ) {
		return;
	}

	UpdateHMDMatrixPose();
}

void LighthouseTracking::EndFrame()
{
}

void LighthouseTracking::AddHapticPulse( vr::ETrackedControllerRole role, float duration )
{
	if( role == vr::TrackedControllerRole_LeftHand )
	{
		m_controllerPulseTimers[0].SetSeconds( duration );
	}
	else if( role == vr::TrackedControllerRole_RightHand )
	{
		m_controllerPulseTimers[1].SetSeconds( duration );
	}
	else
	{
		g_theConsole->Error( "%s - Invalid controller role!", __FUNCTION__  );
	}

}

bool errorShown = false;

void LighthouseTracking::RenderControllerModel( vr::Hmd_Eye nEye )
{
	UNUSED( nEye );
	if( !(g_theLighthouse && g_theLighthouse->IsValid()) ) {
		return;
	}
	if( !m_hideControllerModelTimer.HasElapsed() ) {
		return;
	}

	bool bIsInputAvailable = m_pHMD->IsInputAvailable();

	for( uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++ )
	{
		if( !m_rTrackedDeviceToRenderModel[unTrackedDevice] || !m_rbShowTrackedDevice[unTrackedDevice] )
			continue;

		const vr::TrackedDevicePose_t& pose = m_rTrackedDevicePose[unTrackedDevice];
		if( !pose.bPoseIsValid )
			continue;

		if( !bIsInputAvailable && m_pHMD->GetTrackedDeviceClass( unTrackedDevice ) == vr::TrackedDeviceClass_Controller )
			continue;


		Matrix4 mat = GetControllerModelMatrix( unTrackedDevice );
		Mat44 newMVP = g_theGame->ConvertMatrix4ToMat44( mat );

		// ----- Render Model rendering -----
		g_theRenderer->SetModelMatrix( newMVP );
		g_theRenderer->DrawMesh( m_rTrackedDeviceToRenderModel[ unTrackedDevice ]->m_mesh );

		// -----Render Player HP -----
		if( g_theGame && g_theGame->GetPlayer() )
		{
			Entity* player = g_theGame->GetPlayer();
			constexpr float fontSize = 0.015f;
			Mat44 HealthBarMatrix = newMVP;
			HealthBarMatrix.Translate3D( Vec3( 0.0075f, 0.035f, 0.01f ) );
			std::string playerHealthStr = Stringf( "HP  %i", player->m_health );

			Rgba8 textColor;
			if( player->m_health >= 50 ) { textColor = Rgba8( 106, 176, 76, 255 ); }
			else if( player->m_health <= 50 && player->m_health > 20 ) { textColor = Rgba8( 249, 202, 36, 255 ); }
			else { textColor = Rgba8( 214, 48, 49, 255 ); }

			g_theDebugRenderSystem->DebugAddWorldText( HealthBarMatrix, Vec2( 0.5f, 0.5f ), textColor, 0.f, fontSize, playerHealthStr.c_str() );
		}

		// ----- Render Controller Axes -----
		//Vector4 center = mat * Vector4( 0, 0, 0, 1 );
		//Vec3 centerPos = Vec3( center.x, center.y, center.z );
		
		//Vector4 start = mat * Vector4( 0, 0, -0.02f, 1 );
		//Vector4 end = mat * Vector4( 0, 0, -39.f, 1 );
		//Vec3 startPos = Vec3( start.x, start.y, start.z );
		//Vec3 endPos = Vec3( end.x, end.y, end.z );
	}

	g_theRenderer->BindTexture( nullptr );
}



void LighthouseTracking::SetupControllerPulseTimers()
{
	for( int i = 0; i <= 1; ++i )
	{
		m_controllerPulseTimers[i].SetSeconds( 0.f );
	}
}

void LighthouseTracking::SetupHideControllerModelTimer()
{
	m_hideControllerModelTimer.SetSeconds( 0.f );
}

//-------------------------------------------------------------------------------------------------------------
void LighthouseTracking::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
	m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
	m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );
}

void LighthouseTracking::SetupRenderModels()
{
	memset( m_rTrackedDeviceToRenderModel, 0, sizeof( m_rTrackedDeviceToRenderModel ) );

	if( !m_pHMD )
		return;

	for( uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice )
	{
		if( !m_pHMD->IsTrackedDeviceConnected( unTrackedDevice ) )
			continue;

		SetupRenderModelForTrackedDevice( unTrackedDevice );
	}
}

void LighthouseTracking::SetupRenderModelForTrackedDevice( vr::TrackedDeviceIndex_t unTrackedDeviceIndex )
{
	if( unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount )
		return;

	std::string sRenderModelName = GetTrackedDeviceString( m_pHMD, unTrackedDeviceIndex, vr::Prop_RenderModelName_String );
	GameObject* pRenderModel = FindOrLoadRenderModel( unTrackedDeviceIndex, sRenderModelName.c_str() );
	if( !pRenderModel )
	{
		std::string sTrackingSystemName = GetTrackedDeviceString( m_pHMD, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String );
		g_theConsole->Error( "Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str() );
	}
	else
	{
		m_rTrackedDeviceToRenderModel[unTrackedDeviceIndex] = pRenderModel;
		m_rbShowTrackedDevice[unTrackedDeviceIndex] = true;
	}
}

GameObject* LighthouseTracking::FindOrLoadRenderModel( vr::TrackedDeviceIndex_t unTrackedDeviceIndex, const char* pchRenderModelName )
{
	UNUSED( unTrackedDeviceIndex );

	GameObject* pRenderModel = nullptr;

	if( strcmp( pchRenderModelName, "vr_controller_vive_1_5" ) == 0 )
	{
		pRenderModel = CreateControllerRenderModel();
		m_vecRenderModels.push_back( pRenderModel );
	}

	return pRenderModel;
}

//-------------------------------------------------------------------------------------------------------------
void LighthouseTracking::dprintf( char const* fmt, ... )
{
	va_list args;
	char buffer[2048];

	va_start( args, fmt );
	vsprintf_s( buffer, fmt, args );
	va_end( args );

	//if( g_bPrintf )
	//	printf( "%s", buffer );

	OutputDebugStringA( buffer );
}

//-------------------------------------------------------------------------------------------------------------
// Purpose: Prints out position (x,y,z) and rotation (qw,qx,qy,qz) into the console.
//-------------------------------------------------------------------------------------------------------------
void LighthouseTracking::PrintPositionalData()
{
	// Process SteamVR device states
	for( vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++ )
	{
		if( !m_pHMD->IsTrackedDeviceConnected( unDevice ) )
			continue;

		vr::VRControllerState_t state;
		if( m_pHMD->GetControllerState( unDevice, &state, sizeof( state ) ) )
		{
			vr::TrackedDevicePose_t trackedDevicePose;
			vr::TrackedDevicePose_t trackedControllerPose;
			vr::VRControllerState_t controllerState;
			vr::HmdMatrix34_t poseMatrix;
			vr::HmdVector3_t position;
			vr::HmdQuaternion_t quaternion;
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass( unDevice );

			switch( trackedDeviceClass ) {
			case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
				vr::VRSystem()->GetDeviceToAbsoluteTrackingPose( vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1 );
				// print positional data for the HMD.
				poseMatrix = trackedDevicePose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				position = GetPosition( trackedDevicePose.mDeviceToAbsoluteTracking );
				quaternion = GetRotation( trackedDevicePose.mDeviceToAbsoluteTracking );

				PrintDevicePositionalData( "HMD", poseMatrix, position, quaternion );

				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
				vr::VRSystem()->GetDeviceToAbsoluteTrackingPose( vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1 );
				// print positional data for a general vive tracker.
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
				vr::VRSystem()->GetControllerStateWithPose( vr::TrackingUniverseStanding, unDevice, &controllerState,
					sizeof( controllerState ), &trackedControllerPose );
				poseMatrix = trackedControllerPose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				position = GetPosition( trackedControllerPose.mDeviceToAbsoluteTracking );
				quaternion = GetRotation( trackedControllerPose.mDeviceToAbsoluteTracking );

				auto trackedControllerRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex( unDevice );
				std::string whichHand = "";
				if( trackedControllerRole == vr::TrackedControllerRole_LeftHand )
				{
					whichHand = "LeftHand";
				}
				else if( trackedControllerRole == vr::TrackedControllerRole_RightHand )
				{
					whichHand = "RightHand";
				}

				switch( trackedControllerRole )
				{
				case vr::TrackedControllerRole_Invalid:
					// invalid
					break;

				case vr::TrackedControllerRole_LeftHand:
				case vr::TrackedControllerRole_RightHand:
					PrintDevicePositionalData( whichHand.c_str(), poseMatrix, position, quaternion );

					break;
				}

				break;
			}

		}
	}

}

//-----------------------------------------------------------------------------
// Purpose: Prints the timestamped data in proper format(x,y,z).
//-----------------------------------------------------------------------------
void LighthouseTracking::PrintDevicePositionalData( const char* deviceName, vr::HmdMatrix34_t posMatrix, vr::HmdVector3_t position, vr::HmdQuaternion_t quaternion )
{
	UNUSED( posMatrix );

	LARGE_INTEGER qpc;	// Query Performance Counter for Acquiring high-resolution time stamps.
						// From MSDN: "QPC is typically the best method to use to time-stamp events and 
						// measure small time intervals that occur on the same system or virtual machine.

	QueryPerformanceCounter( &qpc );


	// Print position and quaternion (rotation).
	/*dprintf( "\n%lld, %s, x = %.5f, y = %.5f, z = %.5f, qw = %.5f, qx = %.5f, qy = %.5f, qz = %.5f",
		qpc.QuadPart, deviceName,
		position.v[0], position.v[1], position.v[2],
		quaternion.w, quaternion.x, quaternion.y, quaternion.z );*/

	// Print position and quaternion (rotation) to screen while in debug mode
	if( g_theGame->GetIsDebugRenderingActive() )
	{
		std::string str = Stringf( "%s, x = %.5f, y = %.5f, z = %.5f, qw = %.5f, qx = %.5f, qy = %.5f, qz = %.5f",
			deviceName,
			position.v[0], position.v[1], position.v[2],
			quaternion.w, quaternion.x, quaternion.y, quaternion.z );
		
		if( strcmp( deviceName, "HMD" ) == 0 ) {
			g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.01f, 0.80f, 0.f, 0.f ), Vec2::ZERO, 13.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, str.c_str() );
		}
		else if( strcmp( deviceName, "LeftHand" ) == 0 ) {
			g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.01f, 0.77f, 0.f, 0.f ), Vec2::ZERO, 13.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, str.c_str() );
		}
		else if( strcmp( deviceName, "RightHand" ) == 0  ) {
			g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.01f, 0.74f, 0.f, 0.f ), Vec2::ZERO, 13.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, str.c_str() );
		}
		else {
		}
	}
	
	// Uncomment this if you want to print entire transform matrix that contains both position and rotation matrix.
	//dprintf("\n%lld,%s,%.5f,%.5f,%.5f,x: %.5f,%.5f,%.5f,%.5f,y: %.5f,%.5f,%.5f,%.5f,z: %.5f,qw: %.5f,qx: %.5f,qy: %.5f,qz: %.5f",
	//    qpc.QuadPart, whichHand.c_str(),
	//    posMatrix.m[0][0], posMatrix.m[0][1], posMatrix.m[0][2], posMatrix.m[0][3],
	//    posMatrix.m[1][0], posMatrix.m[1][1], posMatrix.m[1][2], posMatrix.m[1][3],
	//    posMatrix.m[2][0], posMatrix.m[2][1], posMatrix.m[2][2], posMatrix.m[2][3],
	//    quaternion.w, quaternion.x, quaternion.y, quaternion.z);
}

//-------------------------------------------------------------------------------------------------------------
void LighthouseTracking::UpdateHMDMatrixPose()
{
	if( !m_pHMD ) {
		return;
	}

	//BlockHereToSyncWithTheCompositorAndWaitForNextFramePoses
	vr::VRCompositor()->WaitGetPoses( m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	// Inform Compositor
	if( !m_IsCompositorReady )
	{
		m_IsCompositorReady = true;
	}

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for( int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice )
	{
		if( m_rTrackedDevicePose[nDevice].bPoseIsValid )
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4( m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking );
			if( m_rDevClassChar[nDevice] == 0 )
			{
				switch( m_pHMD->GetTrackedDeviceClass( nDevice ) )
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'O'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid )
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd].invert();
	}
	else
	{
		g_theConsole->Printf( "Pose not valid" );
	}
}

bool LighthouseTracking::IsValid() const
{
	return (m_pHMD);
}

std::string LighthouseTracking::GetTrackedDeviceString( vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError )
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if( unRequiredBufferLen == 0 )
		return "";

	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

//-------------------------------------------------------------------------------------------------------------
Matrix4 LighthouseTracking::GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye )
{
	if( !m_pHMD )
		return Matrix4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix( nEye, m_fNearClip, m_fFarClip );

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
		);
}

//-------------------------------------------------------------------------------------------------------------
Matrix4 LighthouseTracking::GetHMDMatrixPoseEye( vr::Hmd_Eye nEye )
{
	if( !m_pHMD )
		return Matrix4();

	vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform( nEye );
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

	return matrixObj.invert();
}

//-------------------------------------------------------------------------------------------------------------
Matrix4 LighthouseTracking::GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye )
{
	Matrix4 matMVP;
	if( nEye == vr::Eye_Left )
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
	}
	else if( nEye == vr::Eye_Right )
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
	}

	return matMVP;
}

Matrix4 LighthouseTracking::GetControllerModelMatrix( vr::TrackedDeviceIndex_t unTrackedDevice )
{
	const Matrix4& matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
	Matrix4 mat = matDeviceToTracking;

	Entity* player = g_theGame->GetPlayer();
	if( player )
	{
		Matrix4 translationMat = translationMat.identity().translate( Vector3( player->m_position.x, player->m_position.y, 0.f ) );
		Matrix4 playerZRotMat = playerZRotMat.identity().rotateZ( player->m_yawDegrees );
		Matrix4 rotYMat = rotYMat.identity().rotateY( -90.f );
		Matrix4 rotZMat = rotZMat.identity().rotateZ( -90.f );

		if( g_theGame->m_locomoiton == eLocomotion::CONTINUOUS )
		{
			mat = translationMat * rotYMat * rotZMat * mat;
		}
		else
		{
			mat = translationMat * playerZRotMat * rotYMat * rotZMat * mat;
		}
	}
	
	return mat;
}

Matrix4 LighthouseTracking::ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t& matPose )
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.f,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.f,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.f,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.f
		);
	return matrixObj;
}

Vec3 LighthouseTracking::GetHMDEulerAngles()
{
	Vec3 eulerAngles;

	for( vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++ )
	{
		if( !m_pHMD->IsTrackedDeviceConnected( unDevice ) )
			continue;

		vr::VRControllerState_t state;
		if( m_pHMD->GetControllerState( unDevice, &state, sizeof( state ) ) )
		{
			vr::TrackedDevicePose_t trackedDevicePose;
			vr::HmdMatrix34_t poseMatrix;
			vr::HmdVector3_t position;
			vr::HmdQuaternion_t quaternion;
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass( unDevice );

			if( trackedDeviceClass == vr::ETrackedDeviceClass::TrackedDeviceClass_HMD ) 
			{
				vr::VRSystem()->GetDeviceToAbsoluteTrackingPose( vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1 );
				// print positional data for the HMD.
				poseMatrix = trackedDevicePose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				position = GetPosition( trackedDevicePose.mDeviceToAbsoluteTracking );
				quaternion = GetRotation( trackedDevicePose.mDeviceToAbsoluteTracking );
				eulerAngles = ToEulerAngles( Quaternion( quaternion.w, quaternion.x, quaternion.y, quaternion.z ) );
			}
		}
	}

	return eulerAngles;
}

Vec3 LighthouseTracking::GetHMDPosition()
{
	Vec3 hmdPosition = Vec3::ZERO;

	for( vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++ ) {
		if( !m_pHMD->IsTrackedDeviceConnected( unDevice ) )
			continue;

		vr::VRControllerState_t state;
		if( m_pHMD->GetControllerState( unDevice, &state, sizeof( state ) ) ) {
			vr::TrackedDevicePose_t trackedDevicePose;
			vr::HmdMatrix34_t poseMatrix;
			vr::HmdVector3_t position;
			vr::HmdQuaternion_t quaternion;
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass( unDevice );

			if( trackedDeviceClass == vr::ETrackedDeviceClass::TrackedDeviceClass_HMD ) {
				vr::VRSystem()->GetDeviceToAbsoluteTrackingPose( vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1 );
				// print positional data for the HMD.
				poseMatrix = trackedDevicePose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				position = GetPosition( trackedDevicePose.mDeviceToAbsoluteTracking );
				hmdPosition = Vec3( position.v[0], position.v[1], position.v[2] );
			}
		}
	}

	return hmdPosition;
}

//-------------------------------------------------------------------------------------------------------------
vr::HmdVector3_t LighthouseTracking::GetPosition( vr::HmdMatrix34_t matrix )
{
	vr::HmdVector3_t vector;

	vector.v[0] = matrix.m[0][3];
	vector.v[1] = matrix.m[1][3];
	vector.v[2] = matrix.m[2][3];

	return vector;
}

//-------------------------------------------------------------------------------------------------------------
vr::HmdQuaternion_t LighthouseTracking::GetRotation( vr::HmdMatrix34_t matrix )
{
	vr::HmdQuaternion_t q;

	q.w = sqrt( fmax( 0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2] ) ) / 2;
	q.x = sqrt( fmax( 0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2] ) ) / 2;
	q.y = sqrt( fmax( 0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2] ) ) / 2;
	q.z = sqrt( fmax( 0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2] ) ) / 2;
	q.x = copysign( q.x, matrix.m[2][1] - matrix.m[1][2] );
	q.y = copysign( q.y, matrix.m[0][2] - matrix.m[2][0] );
	q.z = copysign( q.z, matrix.m[1][0] - matrix.m[0][1] );
	return q;
}

void LighthouseTracking::CreateControllerMesh()
{
	std::vector<Vertex_PCUTBN> verts;
	std::vector<uint> indices;
	char const* filename = "Data/Models/vr_controller_vive_1_5.obj";
	mesh_import_options_t meshImportOptionsData;
	meshImportOptionsData.generate_tangents = true;
	//LoadOBJToVertexArray( verts, indices, filename, meshImportOptionsData );
	LoadModel( verts, indices, filename, meshImportOptionsData );

	GPUMesh* mesh = new GPUMesh( g_theRenderer );
	mesh->UpdateVertices( verts );
	//mesh->UpdateIndices(indices );

	m_controllerGameObject = new GameObject();
	m_controllerGameObject->SetMesh( mesh );
	m_controllerGameObject->m_transform.SetRotationFromPitchRollYawDegrees( 0.f, 90.f, 270.f );
	m_controllerGameObject->m_transform.SetPosition( Vec3( 2.f, 1.5f, 0.5f ) );
}

GameObject* LighthouseTracking::CreateControllerRenderModel()
{
	std::vector<Vertex_PCUTBN> verts;
	std::vector<uint32_t> indices;
	//char const* filename = "Data/Models/vr_controller_vive_1_5.obj";
	char const* filename = "Data/Models/gunforyifan.obj";
	mesh_import_options_t meshImportOptionsData;
	meshImportOptionsData.generate_tangents = true;

	LoadModel( verts, indices, filename, meshImportOptionsData );

	GPUMesh* mesh = new GPUMesh( g_theRenderer );
	mesh->UpdateVertices( verts );
	mesh->UpdateIndices( indices );

	GameObject* pControllerRenderModel = new GameObject();
	pControllerRenderModel->SetMesh( mesh );
	pControllerRenderModel->m_transform.SetRotationFromPitchRollYawDegrees( 0.f, 90.f, 270.f );

	return pControllerRenderModel;
}
