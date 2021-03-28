#pragma once
#include "Game/World.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Core/Delegate.hpp"


//-------------------------------------------------------------------------------------------------------------------------
// constants
//-------------------------------------------------------------------------------------------------------------------------
constexpr int MAX_NUM_LIGHTS = 8; 

//-------------------------------------------------------------------------------------------------------------------------
class GameObject;
class Entity;
class LighthouseTracking;
class Matrix4;

//-------------------------------------------------------------------------------------------------------------------------
enum class eVRControllerTeleportingState
{
	INVALID = -1,
	LEFT_HAND_TELEPORTING,
	RIGHT_HAND_TELEPORTING,
};

//-------------------------------------------------------------------------------------------------------------
enum class eLightType 
{
	DIRECTION_LIGHT,
	POINT_LIGHT,
	SPOT_LIGHT,

	NUM_LIGHT_TYPES
};

enum class eLightBehavior
{
	FOLLOW_CAMERA,
	STAY,
};

enum class eLocomotion
{
	TELEPORTATION,
	CONTINUOUS
};

struct LightMaster 
{
	int lightIdx = 0;
	light_constants_t lightConstants;
	eLightBehavior lightBehaviors[MAX_NUM_LIGHTS];
};


class Game
{
public:
	Game();
	~Game();

	void StartUp();
	void ShutDown();
	void EndFrame();
	void Update( float deltaSeconds );
	void Render();
	void RenderUI() const;

	void UpdateKeyFunctions();
	void UpdateCursorPosition();
	void AdjustAmbientIntensity();
	void ToggleLightAttenuationMode();
	void UpdateLightKeyFunctions();
	void SetLightType( light_t& light, eLightType lightType );
	void SetLightType( uint idx, eLightType lightType );
	std::string GetLightBehaviorAsString( eLightBehavior behavior );
	void UpdateMaterialConstants();
	void AddCameraInfoScreenText();
	void UpdateLightBehaviors();

	// Accessors
	bool GetIsDebugRenderingActive() { return m_isDebugRenderingActive; }

	Mat44 ConvertMatrix4ToMat44( Matrix4 other );

	// -----VR-----
	void		DectectVRControllerRaycast();
	void		UpdateTeleportingState( std::string const& controllerRole );
	void		TeleportPlayer( std::string const& controllerRole );
	void		Fire( std::string const& controllerRole );
	void		PlayerRequestMoveForward();
	void		PlayerRequestMoveBackward();
	Entity*		GetPlayer();

private:
	void CreateQuadTBN();
	void CreateCubeTBN( Vec3 min );
	void CreateSphereTBN();
	void CreateCubeMesh();
	void CreateSphereMesh();
	void CreateIcoSphereMesh();
	void RotateCube( float deltaSeconds );
	void RotateSpheres( float deltaSeconds );
	void InitializeSpotLight();
	Vec3 ConvertRgba8ToVec3( Rgba8 rgba8 ) const;
	void AddHUDScreenRender();
	void ProcessInput( float deltaSeconds );

	// obj file
	void TestOBJLoader();
	void CreateFighterMesh();
	void CreateCane();

	void InitDissolveMaterial();
	void InitLitMaterial();
	void InitWorldOpaqueMaterial();
	void InitStoneMaterial();

	void InitWorldCameras();
	void InitUICamera();

	bool InitSoundEffectFiles();
	void CalculateBezierCurve( Vec3& startPoint, Vec3& endPoint, int vertexCount, std::vector<Vec3>& pointList, const Matrix4& controllerModelMatrix );
	Vec3 LinePlaneIntersection( Vec3 planePoint, Vec3 planeNormal, Vec3 linePoint, Vec3 lineDirection );


public:
	// World
	World* m_theWorld = nullptr;

	// Player
	//Entity* m_player = nullptr;

	bool m_isUpPressed     = false;
	bool m_isDownPressed   = false;
	bool m_isLeftPressed   = false;
	bool m_isRightPressed  = false;

	Camera m_worldCameraLeft;
	Camera m_worldCameraRight;
	Camera m_uiCamera;

	GPUMesh* m_meshCube = nullptr;
	GPUMesh* m_meshSphere = nullptr;
	GPUMesh* m_meshIcoSphere = nullptr;

	// TBN mesh
	GPUMesh* m_meshQuadTBN = nullptr;
	

	std::vector<GameObject*> m_gameObejcts;
	std::vector<GameObject*> m_gameObejctsTBN;

	Physics2D* m_physics = nullptr;

	// Lights
	light_t m_pointLight;
	LightMaster m_lightMaster;

	Rgba8 m_ambientLightColor = Rgba8::WHITE;
	float m_ambientIntensity = 0.01f;

	// Materials
	Material* m_litMaterial = nullptr;
	Material* m_dissolveMaterial = nullptr;
	Material* m_worldOpaqueMaterial = nullptr;
	Material* m_stoneMaterial = nullptr;

public:
	Vec2 m_cursorPosition = Vec2::ZERO;
	AABB2 m_cursorBox = AABB2::ZERO_TO_ONE;

	// Clear Screen
	Rgba8 m_colorClearScreen = Rgba8::BLACK;

	int m_shaderIndex = 0;
	float m_gamma = 2.2f;

	bool m_isDebugRenderingActive = false;

	// ----- VR -----
	//bool m_isTeleporting = false;
	eVRControllerTeleportingState m_teleportingState;
	Vec3 m_teleportDestination;
	bool m_isTouchpadTopButtonDown = false;
	bool m_isTouchpadBottomButtonDown = false;

	eLocomotion m_locomoiton = eLocomotion::TELEPORTATION;

	GPUMesh* m_bulletMesh = nullptr;
};