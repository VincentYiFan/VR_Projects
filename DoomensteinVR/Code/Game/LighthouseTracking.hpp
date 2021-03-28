#pragma once

#include "Engine/Platform/Window.hpp"
#include "Engine/Core/Timer.hpp"
#include <openvr.h>
#include <vector>
#include "Matrices.h"

//-------------------------------------------------------------------------------------------------------------
class  Texture;
class  GameObject;
struct Vec3;
//-------------------------------------------------------------------------------------------------------------

class LighthouseTracking
{
public:
	~LighthouseTracking();
	LighthouseTracking();

	bool BInit();
	bool BInitCompositor();

	void StartUp( Window* window );
	void ShutDown();

	void RunMainLoop();
	bool HandleInput();
	void ProcessVREvent( vr::VREvent_t const& event );
	void DealWithButtonEvent( vr::VREvent_t const& event );
	void RenderFrame();
	void EndFrame();

	void AddHapticPulse( vr::ETrackedControllerRole role, float duration );

	void RenderControllerModel( vr::Hmd_Eye nEye );

	void SetupControllerPulseTimers();
	void SetupHideControllerModelTimer();	// After pressed rotate button on the touch pad, hide the model for a short amount of time
	void SetupCameras();
	void SetupRenderModels();
	void SetupRenderModelForTrackedDevice( vr::TrackedDeviceIndex_t unTrackedDeviceIndex );
	GameObject* FindOrLoadRenderModel( vr::TrackedDeviceIndex_t unTrackedDeviceIndex, const char* pchRenderModelName );

	void dprintf( char const* fmt, ... );

	void PrintPositionalData();
	void PrintDevicePositionalData( const char * deviceName, vr::HmdMatrix34_t posMatrix, vr::HmdVector3_t position, vr::HmdQuaternion_t quaternion );

	void UpdateHMDMatrixPose();

	bool IsValid() const;

	std::string GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL );

	Matrix4		GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye );
	Matrix4		GetHMDMatrixPoseEye( vr::Hmd_Eye nEye );
	Matrix4		GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye );
	Matrix4		GetControllerModelMatrix( vr::TrackedDeviceIndex_t unTrackedDevice );
	Matrix4		ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t& matPose );

	Vec3		GetHMDEulerAngles();
	Vec3		GetHMDPosition();
	//-------------------------------------------------------------------------------------------------------------
	GameObject* GetControllerGameObject() { return m_controllerGameObject; }
	void		CreateControllerMesh();

	GameObject* CreateControllerRenderModel();

private:
	// Position and rotation of pose
	vr::HmdVector3_t		GetPosition( vr::HmdMatrix34_t matrix );
	vr::HmdQuaternion_t		GetRotation( vr::HmdMatrix34_t matrix );

	

//private:
public:
	Window*						m_theWindow = nullptr;
	GameObject*					m_controllerGameObject = nullptr; // #ToDo: Delete Later.

	std::vector< GameObject* > m_vecRenderModels;
	GameObject* m_rTrackedDeviceToRenderModel[ vr::k_unMaxTrackedDeviceCount ];

	vr::IVRSystem* m_pHMD = nullptr;
	vr::IVRRenderModels* m_pRenderModels = nullptr;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	bool m_rbShowTrackedDevice[ vr::k_unMaxTrackedDeviceCount ];

	int m_iValidPoseCount = 0;

	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class

	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

	float m_fScale = 0.f;
	float m_fScaleSpacing = 0.f;
	float m_fNearClip = 0.f;
	float m_fFarClip = 0.f;

	Matrix4 m_mat4HMDPose;
	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;

	bool	m_IsCompositorReady = false;

	Timer m_controllerPulseTimers[2]; // left, right controller
	Timer m_hideControllerModelTimer;

	float m_previousHMDYawDegrees = 0.f;
};
