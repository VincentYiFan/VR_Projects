#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapRegionType.hpp"
#include "Game/MapMaterial.hpp"
#include "Game/TileMap.hpp"
#include "Game/EntityDef.hpp"
#include "Game/Actor.hpp"
#include "Game/Projectile.hpp"
#include "Game/LighthouseTracking.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/ShaderState.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Physics/GameObject.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include <string>
#include <vector>

Game* g_theGame = nullptr;
RandomNumberGenerator* g_rng = nullptr; 
BitmapFont* g_theFont = nullptr;
eBillboardMode g_debugBillboardMode = eBillboardMode::BILLBOARD_MODE_CAMERA_FACING_XY;

//#define UI_CAMERA_ENABLED
#define LIGHTS_ENABLED

Game::Game()
	:m_teleportingState( eVRControllerTeleportingState::INVALID )
{	
}

Game::~Game()
{
	m_teleportingState = eVRControllerTeleportingState::INVALID;

	delete m_theWorld;
	m_theWorld = nullptr;
}

void Game::StartUp()
{
	// Setup physics2D system
	m_physics = new Physics2D();

	// Setup Random number Generator
	g_rng = new RandomNumberGenerator();

	// Load shader state definitions from file
	ShaderState::SetupFromXML( "Data/Definitions/ShaderState.xml" );
	g_theConsole->PrintString( Rgba8::GREEN, "ShaderStateDefs Loaded!" );

	// Load material definitions from file
	Material::LoadDefinitions( g_theRenderer, "Data/Definitions/Materials.xml" );
	g_theConsole->PrintString( Rgba8::GREEN, "MaterialDefs Loaded!" );

	// Load Entity Types Definitions
	EntityDef::LoadDefinitions( "Data/Definitions/EntityTypes.xml" );
	g_theConsole->PrintString( Rgba8::GREEN, "EntityType Definitions Loaded!" );

	// Setup Bitmap Font
	g_theFont = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont" );

	// Load audio files
	if( !InitSoundEffectFiles() )
	{
		g_theConsole->Error( "%s - Failed to load audio files", __FUNCTION__ );
	}

	// Initialize Materials
	InitLitMaterial();
	InitDissolveMaterial();
	InitWorldOpaqueMaterial();
	InitStoneMaterial();

	// Initialize World cameras
	InitWorldCameras();

#ifdef UI_CAMERA_ENABLED
	// Setup UI Camera
	InitUICamera();
#endif

	// Initialize first light
	m_lightMaster.lightConstants.lights[0].position = Vec3( 4.f, 5.f, 1.25f );
	m_lightMaster.lightConstants.lights[0].direction = Vec3(0.f,0.f,-1.f);
	m_lightMaster.lightConstants.lights[0].intensity = 1.f;
	m_lightMaster.lightBehaviors[0] = eLightBehavior::STAY;


	m_lightMaster.lightConstants.lights[1].position = Vec3( 6.5f, 2.5f, 1.5f );
	m_lightMaster.lightConstants.lights[1].direction = Vec3(0.f,0.f,-1.f);
	m_lightMaster.lightConstants.lights[1].intensity = 0.75f;
	m_lightMaster.lightBehaviors[1] = eLightBehavior::STAY;

	m_lightMaster.lightConstants.lights[2].position = Vec3( 11.5f, 5.f, 1.5f );
	m_lightMaster.lightConstants.lights[2].direction = Vec3( 0.f, 0.f, -1.f );
	m_lightMaster.lightConstants.lights[2].intensity = 1.f;
	m_lightMaster.lightBehaviors[2] = eLightBehavior::STAY;

	m_lightMaster.lightConstants.lights[4].position = Vec3( 6.f, 2.5f, 1.25f );
	m_lightMaster.lightConstants.lights[4].direction = Vec3( 0.f, 0.f, -1.f );
	m_lightMaster.lightConstants.lights[4].intensity = 0.75f;
	m_lightMaster.lightBehaviors[4] = eLightBehavior::STAY;

	// make second light a spot light
	//InitializeSpotLight();

	// Setup World Camera's World Basis Convention
	const std::string coordinateStr = g_gameConfigBlackboard.GetValue( "coordinate", "" );
	if( coordinateStr == "XFORWARD_YLEFT_ZUP" )
	{
		m_worldCameraLeft.m_convention = eWorldBasisConvention::WORLD_BASIS_XFORWARD_YLEFT_ZUP;
		m_worldCameraRight.m_convention = eWorldBasisConvention::WORLD_BASIS_XFORWARD_YLEFT_ZUP;
	}

	MapMaterial::LoadDefinitions( "Data/Definitions/MapMaterialTypes.xml" );
	MapRegionType::LoadDefinitions( "Data/Definitions/MapRegionTypes.xml" );

	// Setup the World
	m_theWorld = new World( m_worldCameraLeft, m_worldCameraRight );

	// Initialize Player
	if( GetPlayer() != nullptr )
	{
		m_teleportDestination = Vec3( GetPlayer()->m_position, 0.f );
	}

	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint32_t> indices;
	mesh_import_options_t options;
	LoadModel( vertices, indices, "Data/Models/Bullet.obj", options );
	m_bulletMesh = new GPUMesh( g_theRenderer );
	m_bulletMesh->UpdateVertices( vertices );
	m_bulletMesh->UpdateIndices( indices );
}

void Game::ShutDown()
{
	// should for loop through s_definitions instead to delete
	delete m_dissolveMaterial;
	m_dissolveMaterial = nullptr;

	delete m_litMaterial;
	m_litMaterial = nullptr;

	delete m_worldOpaqueMaterial;
	m_worldOpaqueMaterial = nullptr;

	delete m_stoneMaterial;
	m_stoneMaterial = nullptr;

	delete m_physics; 
	m_physics = nullptr;

	delete m_meshCube;
	m_meshCube = nullptr;

	delete m_meshSphere;
	m_meshSphere = nullptr;

	delete m_meshIcoSphere;
	m_meshIcoSphere = nullptr;

	for( int idx = 0; idx < (int)m_gameObejctsTBN.size(); ++idx ) 
	{
		delete m_gameObejctsTBN[idx];
		m_gameObejctsTBN[idx] = nullptr;
	}

	delete m_bulletMesh;
	m_bulletMesh = nullptr;

	m_worldCameraLeft.CleanUBO();
	m_worldCameraRight.CleanUBO();

#ifdef UI_CAMERA_ENABLED
	m_uiCamera.CleanUBO();
#endif
}

void Game::EndFrame()
{
	m_physics->EndFrame();
}

void Game::Update( float deltaSeconds )
{
	m_worldCameraLeft.SetClearMode( CLEAR_COLOR_BIT, m_colorClearScreen );
	m_worldCameraRight.SetClearMode( CLEAR_COLOR_BIT, m_colorClearScreen );

	UpdateCursorPosition();
	UpdateKeyFunctions();
	AdjustAmbientIntensity();
	AddCameraInfoScreenText();
	DectectVRControllerRaycast();
	UpdateLightBehaviors();

	// FPS counter
	std::string str = Stringf( "%.2f ms/frame (%i FPS)", deltaSeconds * 100.f, (int)(1.f/deltaSeconds) );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.83f, 0.98f, 0.f, 0.f ), Vec2::ZERO, 12.5f, Rgba8::GREEN, Rgba8::GREEN, 0.f, str.c_str() );

	//Mat44 cameraModel = m_worldCameraLeft.m_transform.ToMatrix();
	//m_lightMaster.lightConstants.lights[0].direction = cameraModel.TransformVector3D( Vec3(0,0,-1) ).GetNormalized(); 

	m_theWorld->Update( deltaSeconds );

	// Snap camera to player entity's current eye position and orientation
	Entity* player = GetPlayer();
	if( player ) {	
		constexpr float eyeHeightOffset = 0.f;
		m_worldCameraLeft.m_transform.SetPosition( Vec3( player->m_position, eyeHeightOffset ) );
		//m_worldCameraLeft.m_transform.m_orientationYaw = player->m_yawDegrees;

		m_worldCameraRight.m_transform.SetPosition( Vec3( player->m_position, eyeHeightOffset ) );
		//m_worldCameraRight.m_transform.m_orientationYaw = player->m_yawDegrees;

		//-------------------------------------------------------------------------------------------------------------
		if( g_theLighthouse->IsValid() ) {
			Vec3 hmdEulerAngles = g_theLighthouse->GetHMDEulerAngles();
			hmdEulerAngles = Vec3( ConvertRadiansToDegrees( hmdEulerAngles.x ),
				ConvertRadiansToDegrees( hmdEulerAngles.x ),
				ConvertRadiansToDegrees( hmdEulerAngles.x ) );

			if( m_locomoiton == eLocomotion::CONTINUOUS ) {
				player->m_yawDegrees = hmdEulerAngles.x;
			}
		}
		//-------------------------------------------------------------------------------------------------------------
		
		if( m_isTouchpadTopButtonDown )
		{
			TileMap* tileMap = dynamic_cast<TileMap*> (m_theWorld->m_currentMap);
			if( tileMap )
			{
				Vec2 disp = player->GetForwardVector() * 0.0075f; // should be HMD's forward dir
				IntVec2 tileCoord = tileMap->GetTileCoordsForWorldPosition( player->m_position + disp );
				bool isLocationValid = !tileMap->IsTileSolid( tileCoord );
				if( isLocationValid )
				{
					player->m_position += disp;
				}
			}
		}
		else if( m_isTouchpadBottomButtonDown )
		{
			TileMap* tileMap = dynamic_cast<TileMap*> (m_theWorld->m_currentMap);
			if( tileMap )
			{
				Vec2 disp = player->GetForwardVector() * 0.0075f; // should be HMD's forward dir
				IntVec2 tileCoord = tileMap->GetTileCoordsForWorldPosition( player->m_position + disp );
				bool isLocationValid = !tileMap->IsTileSolid( tileCoord );
				if( isLocationValid )
				{
					player->m_position -= disp;
				}
			}
		}
	}
}

void Game::UpdateKeyFunctions()
{
	if( g_theInput->WasKeyJustPressed( KEY_ESC ) )
	{
		if( !g_theConsole->IsOpen() ) {
			g_theApp->HandleQuitRequested();
		}
		else {
			g_theConsole->SetIsOpen( false );
		}
	}

	if( g_theInput->WasKeyJustPressed( KEY_F1 ) )
	{
		// Toggle debug mode
		m_isDebugRenderingActive = !m_isDebugRenderingActive;
	}

	if( g_theInput->WasKeyJustPressed( KEY_TLIDE ) ) 
	{
		// Toggle DevConsole
		if( g_theConsole->IsOpen() )
		{
			g_theConsole->SetIsOpen( false );
		}
		else
		{
			g_theConsole->SetIsOpen( true );
		}
	}

	if( g_theInput->WasKeyJustPressed( 'L' ) )
	{
		if( m_locomoiton == eLocomotion::TELEPORTATION )
		{
			m_locomoiton = eLocomotion::CONTINUOUS;
			Vec3 hmdEulerAngles = g_theLighthouse->GetHMDEulerAngles();
			hmdEulerAngles = Vec3( ConvertRadiansToDegrees( hmdEulerAngles.x ),
				ConvertRadiansToDegrees( hmdEulerAngles.x ),
				ConvertRadiansToDegrees( hmdEulerAngles.x ) );

			m_worldCameraLeft.m_transform.m_orientationYaw = hmdEulerAngles.x;
			m_worldCameraRight.m_transform.m_orientationYaw = hmdEulerAngles.x;
		}
		else
		{
			m_locomoiton = eLocomotion::TELEPORTATION;
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	g_theRenderer->BindTexture( nullptr );
}

//-------------------------------------------------------------------------------------------------------------
void Game::Render()
{
	// setup lights for the scene
	g_theRenderer->SetAmbientLight( m_ambientLightColor, m_ambientIntensity );

	for( int idx = 0; idx < MAX_NUM_LIGHTS; ++idx ) {
		g_theRenderer->EnableLight( idx, m_lightMaster.lightConstants.lights[idx] );
	}

	if( g_theLighthouse->IsValid() )
	{
		g_theLighthouse->RenderFrame();

		// Set Cameras' Projection Matrix
		Mat44 currentLeftEyeProjectionMat = ConvertMatrix4ToMat44( g_theLighthouse->GetCurrentViewProjectionMatrix( vr::Eye_Left ) );
		m_worldCameraLeft.SetProjectionMatrix( currentLeftEyeProjectionMat );

		Mat44 currentRightEyeProjectionMat = ConvertMatrix4ToMat44( g_theLighthouse->GetCurrentViewProjectionMatrix( vr::Eye_Right ) );
		m_worldCameraRight.SetProjectionMatrix( currentRightEyeProjectionMat );
	}

	//-------------------------------------------------------------------------------------------------------------
	// -----Render right world camera -----
	//-------------------------------------------------------------------------------------------------------------
	m_worldCameraRight.SetColorTarget( g_theRenderer->GetBackBuffer() );
	g_theRenderer->BeginCamera( m_worldCameraRight );

	g_theRenderer->BindMaterial( m_stoneMaterial );
	m_theWorld->Render( eCameras::RIGHT_CAMERA );
	
	// Render Controller(Gun) Model
	g_theRenderer->BindMaterial( m_litMaterial );
	g_theLighthouse->RenderControllerModel( vr::EVREye::Eye_Right );

	g_theRenderer->BindMaterial( m_worldOpaqueMaterial );
	g_theDebugRenderSystem->DebugRenderWorldToCamera( m_worldCameraRight );

	g_theRenderer->EndCamera( m_worldCameraRight );

	Texture noMSAARightEyeTex = Texture( g_theRenderer->GetBackBuffer() );
	g_theRenderer->m_context->ResolveSubresource( noMSAARightEyeTex.GetHandle(), D3D11CalcSubresource(0, 0, 1), g_theRenderer->GetBackBuffer()->GetHandle(), D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R8G8B8A8_UNORM );
	//-------------------------------------------------------------------------------------------------------------
	// -----Render left world camera -----
	//-------------------------------------------------------------------------------------------------------------
	m_worldCameraLeft.SetColorTarget( g_theRenderer->GetBackBuffer() );
	g_theRenderer->BeginCamera( m_worldCameraLeft );

	// Render the World
	g_theRenderer->BindMaterial( m_stoneMaterial );
	m_theWorld->Render( eCameras::LEFT_CAMERA );

	// Render Controller(Gun) Model
	g_theRenderer->BindMaterial( m_litMaterial );
	g_theLighthouse->RenderControllerModel( vr::EVREye::Eye_Left );

	g_theRenderer->BindMaterial( m_worldOpaqueMaterial );
	g_theDebugRenderSystem->DebugRenderWorldToCamera( m_worldCameraLeft );

	g_theRenderer->EndCamera( m_worldCameraLeft );

	Texture noMSAALeftEyeTex = Texture( g_theRenderer->GetBackBuffer() );
	g_theRenderer->m_context->ResolveSubresource( noMSAALeftEyeTex.GetHandle(), D3D11CalcSubresource( 0, 0, 1 ), g_theRenderer->GetBackBuffer()->GetHandle(), D3D11CalcSubresource( 0, 0, 1 ), DXGI_FORMAT_R8G8B8A8_UNORM );
	//-------------------------------------------------------------------------------------------------------------
	// Submit both eyes texture to the vr compositor
	if( g_theLighthouse->IsValid() )
	{
		//g_theLighthouse->RenderFrame();
		
		vr::Texture_t leftEyeTexture ={ noMSAALeftEyeTex.GetHandle(), vr::TextureType_DirectX, vr::ColorSpace_Auto };
		vr::Texture_t rightEyeTexture ={ noMSAARightEyeTex.GetHandle(), vr::TextureType_DirectX, vr::ColorSpace_Auto };

		vr::EVRCompositorError error1 = vr::VRCompositor()->Submit( vr::Eye_Left, &leftEyeTexture );
		if( error1 ) {
			g_theConsole->Error( "Error: Call to VRCompositor()->Submit left eye texture failed, error= %d", error1 );
		}

		vr::EVRCompositorError error2 = vr::VRCompositor()->Submit( vr::Eye_Right, &rightEyeTexture );
		if( error2 ) {
			g_theConsole->Error( "Error: Call to VRCompositor()->Submit right eye texture failed, error= %d", error2 );
		}
	}
		  
	g_theDebugRenderSystem->DebugRenderScreenTo( g_theRenderer->GetBackBuffer() );


#ifdef UI_CAMERA_ENABLED
	g_theRenderer->BeginCamera( m_uiCamera );
	g_theRenderer->EndCamera( m_uiCamera );
#endif
}

//-------------------------------------------------------------------------------------------------------------
void Game::CreateQuadTBN()
{
	m_meshQuadTBN = new GPUMesh( g_theRenderer );

	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;

	AppendQuadToVerts( vertices, indices, Vec3( -1.f, -1.f, -10.f ), Vec3( 1.f, -1.f, -10.f ), Vec3( 1.f, 1.f, -10.f ), Vec3( -1.f, 1.f, -10.f ), Rgba8::WHITE );

	m_meshQuadTBN->UpdateVertices( vertices );
	m_meshQuadTBN->UpdateIndices( indices );

	GameObject* quadTBN = new GameObject();
	quadTBN->SetMesh( m_meshQuadTBN );
	m_gameObejctsTBN.push_back( quadTBN );
}

void Game::CreateCubeTBN( Vec3 min )
{
	GPUMesh* m_meshCubeTBN = new GPUMesh( g_theRenderer );

	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;

	AABB3 aabb3 = AABB3( Vec3( -0.5f, -0.5f, -0.5f ), Vec3( 0.5f, 0.5f, 0.5f ) );
	AppendIndexedCubeWithNormalToVerts( vertices, indices, aabb3 );

	m_meshCubeTBN->UpdateVertices( vertices );
	m_meshCubeTBN->UpdateIndices( indices );

	// Create first TBN Cube
	GameObject* cubeTBN = new GameObject();
	cubeTBN->SetMesh( m_meshCubeTBN );

	// Calculate Center from Min (should be positive)
	Vec3 centerPos = min + Vec3( 0.5f, 0.5f, 0.5f );

	cubeTBN->m_transform.SetPosition( centerPos );
	cubeTBN->m_transform.SetRotationFromPitchRollYawDegrees( 90.f, 0.f, 0.f );

	// Push back into the GameObjects Array 
	m_gameObejctsTBN.push_back( cubeTBN );
}

void Game::CreateSphereTBN()
{
	GPUMesh* meshSphereTBN = new GPUMesh( g_theRenderer );

	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;

	AddUVSphereToIndexedVertexArrayTBN( vertices, indices, Vec3::ZERO/*Vec3( -5.f, 0.f, -5.f )*/, 1.5f, 20, 20, Rgba8::WHITE );

	meshSphereTBN->UpdateVertices( vertices );
	meshSphereTBN->UpdateIndices( indices );

	GameObject* sphereTBN = new GameObject();
	sphereTBN->SetMesh( meshSphereTBN );
	sphereTBN->m_transform.SetPosition( Vec3( -5.f, 0.0f, -10.f ) );
	m_gameObejctsTBN.push_back( sphereTBN );
}

void Game::CreateCubeMesh()
{
	// GPUMesh
	m_meshCube = new GPUMesh( g_theRenderer );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AABB3 aabb3 = AABB3( Vec3( -1.f, -1.f, -1.f ), Vec3( 1.f, 1.f, 1.f ) );
	//aabb3.Translate( Vec3( 1.f, 0.5f, -12.f ) );
	AppendIndexedCubeToVerts( vertices, indices, aabb3 );

	m_meshCube->UpdateVertices( vertices );
	m_meshCube->UpdateIndices( indices );

	GameObject* cube = new GameObject();
	cube->m_mesh = m_meshCube;
	////cube->m_transform.Translate( Vec3( 1.f, 0.5f, -12.f ) );

	const int idx = (int)m_gameObejcts.size();
	m_gameObejcts.push_back( cube );
	m_gameObejcts[idx]->m_transform.SetPosition( Vec3( 1.f, 0.5f, -12.f ) );
}

void Game::CreateSphereMesh()
{
	// GPUMesh
	m_meshSphere = new GPUMesh( g_theRenderer );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AddUVSphereToIndexedVertexArray( vertices, indices, Vec3( 5.f, 5.f, -15.f ), 2.f, 20, 20, Rgba8::WHITE );

	m_meshSphere->UpdateVertices( vertices );
	m_meshSphere->UpdateIndices( indices );


	const float perSliceDegrees = (float)(360 / NUM_SPHERE);
	for( float currentDegrees = 0; currentDegrees <= 360.f; currentDegrees += perSliceDegrees )
	{
		GameObject* newSphere = new GameObject();
		newSphere->m_mesh = m_meshSphere;

		int goIndex = (int)m_gameObejcts.size();
		m_gameObejcts.push_back( newSphere );

		float radius = 10.f;
		m_gameObejcts[goIndex]->m_transform.SetPosition( Vec3( radius * CosDegrees( currentDegrees ), radius * SinDegrees( currentDegrees ), -12.f ) );
	}
}

void Game::CreateIcoSphereMesh()
{
	m_meshIcoSphere = new GPUMesh( g_theRenderer );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AddIcosphereToVerts( vertices, indices, Vec3::ZERO, 2.f, 1, Rgba8::WHITE );

	m_meshIcoSphere->UpdateVertices( vertices );
	m_meshIcoSphere->UpdateIndices( indices );

	GameObject* IcoSphere = new GameObject();
	IcoSphere->m_mesh = m_meshCube;
	m_gameObejcts.push_back( IcoSphere );
}


void Game::RotateCube( float deltaSeconds )
{
	if( m_gameObejctsTBN.size() > 0 ){
		m_gameObejctsTBN[0]->m_transform.AddRotationFromPitchRollYawDegrees( deltaSeconds * 25.f, deltaSeconds * 50.f, 0.f );
	}
}

void Game::RotateSpheres( float deltaSeconds )
{
	for( int i = 1; i < (int)m_gameObejcts.size(); ++i )
	{
		m_gameObejcts[i]->m_transform.AddRotationFromPitchRollYawDegrees( 0, 0, deltaSeconds * 1000.f );
		Vec2 xy = Vec2( m_gameObejcts[i]->m_transform.m_position.x, m_gameObejcts[i]->m_transform.m_position.y );
		xy.RotateDegrees( deltaSeconds * 50.f );
		m_gameObejcts[i]->m_transform.SetPosition( Vec3( xy.x, xy.y, m_gameObejcts[i]->m_transform.m_position.z ) );
	}
}

void Game::InitializeSpotLight()
{
	m_lightMaster.lightConstants.lights[1].intensity = 2.f;
	m_lightMaster.lightConstants.lights[1].dot_inner_angle = CosDegrees(10.f);
	m_lightMaster.lightConstants.lights[1].dot_outer_angle = CosDegrees(15.f);
	m_lightMaster.lightConstants.lights[1].direction = Vec3(0.f,0.f,-1.f);
	m_lightMaster.lightConstants.lights[1].color = Vec3(1.f, 0.f, 1.f);
}

Vec3 Game::ConvertRgba8ToVec3( Rgba8 rgba8 ) const
{
	Vec3 vec3;
	vec3.x = (float)(rgba8.r / 255);
	vec3.y = (float)(rgba8.g / 255);
	vec3.z = (float)(rgba8.b / 255);
	//finalColor.w = (float)(color.a / 255);
	return vec3;
}

void Game::AddHUDScreenRender()
{
	// Add HUD element
	Texture* hudTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Hud_Base.png" );
	float hudWidth = g_theRenderer->m_swapchain->GetBackBuffer()->GetDimensions().x;
	float hudHeight =  hudWidth / hudTexture->GetAspect();
	AABB2 hudBounds = AABB2( Vec2::ZERO, Vec2( hudWidth, hudHeight ) );
	g_theDebugRenderSystem->DebugAddScreenTexturedQuad( hudBounds, hudTexture );

	// Add Gun View Model
	Texture* gunViewModelSpriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/ViewModelsSpriteSheet_8x8.png" );
	SpriteSheet* gunViewModelSpriteSheet = new SpriteSheet( *gunViewModelSpriteSheetTexture, IntVec2( 8, 8 ) );

	float length = g_gameConfigBlackboard.GetValue( "weaponViewModelSize ", 940.f );
	Vec2 gunMins = Vec2( (hudWidth - length) * 0.5f, hudHeight );
	Vec2 gunMaxs = gunMins + Vec2( length, length );
	AABB2 gunBounds = AABB2( gunMins, gunMaxs );

	Vec2 uvMins;
	Vec2 uvMaxs;
	gunViewModelSpriteSheet->GetSpriteUVs( uvMins, uvMaxs, 0 );

	g_theDebugRenderSystem->DebugAddScreenTexturedQuad( gunBounds, gunViewModelSpriteSheetTexture, AABB2( uvMins, uvMaxs ), Rgba8::WHITE, 0.f );
}

void Game::ProcessInput( float deltaSeconds )
{
	Vec3 movement = Vec3::ZERO;

	if( g_theInput->IsKeyPressed( KEY_D ) ) { movement.y -= CAMERA_MOVEMENT_SPEED * deltaSeconds; }
	if( g_theInput->IsKeyPressed( KEY_A ) ) { movement.y += CAMERA_MOVEMENT_SPEED * deltaSeconds; }
		
	if( g_theInput->IsKeyPressed( KEY_Q ) ) { movement.z += CAMERA_MOVEMENT_SPEED * deltaSeconds; }
	if( g_theInput->IsKeyPressed( KEY_E ) ) { movement.z -= CAMERA_MOVEMENT_SPEED * deltaSeconds; }

	if( g_theInput->IsKeyPressed( KEY_W ) ) { movement.x += CAMERA_MOVEMENT_SPEED * deltaSeconds; }
	if( g_theInput->IsKeyPressed( KEY_S ) ) { movement.x -= CAMERA_MOVEMENT_SPEED * deltaSeconds; }


	// Clamp Pitch from -90 to 90 degrees
	m_worldCameraLeft.m_transform.m_orientationPitch = Clamp( m_worldCameraLeft.m_transform.m_orientationPitch, -89.9f, 89.f );
	m_worldCameraRight.m_transform.m_orientationPitch = Clamp( m_worldCameraRight.m_transform.m_orientationPitch, -89.9f, 89.f );

	//if( !g_theLighthouse->IsValid() )
	//{
	//	m_worldCameraLeft.m_transform.AddRotationFromPitchRollYawDegrees( deltaSeconds* CAMERA_MOVEMENT_SPEED * g_theInput->m_relativeMovement.y, 0.f, 0.f );
	//	m_worldCameraRight.m_transform.AddRotationFromPitchRollYawDegrees( deltaSeconds* CAMERA_MOVEMENT_SPEED * g_theInput->m_relativeMovement.y, 0.f, 0.f );
	//}

	// Apply Transform Matrix to World Camera
	movement = movement.GetRotatedAboutZDegrees( m_worldCameraLeft.m_transform.m_orientationYaw );

	if( GetPlayer() )
	{
		//m_player->m_position += Vec2( movement.x, movement.y );
		//m_player->m_yawDegrees += -deltaSeconds* CAMERA_MOVEMENT_SPEED * g_theInput->m_relativeMovement.x;
	}
	else
	{
		// Add Rotation from Pitch Roll Yaw Degrees for Left Camera
		m_worldCameraLeft.m_transform.AddRotationFromPitchRollYawDegrees( 
			0.f,
			0.f,
			-deltaSeconds* CAMERA_MOVEMENT_SPEED * g_theInput->m_relativeMovement.x );

		m_worldCameraLeft.m_transform.Translate( movement );

		// Add Rotation from Pitch Roll Yaw Degrees for Right Camera
		m_worldCameraRight.m_transform.AddRotationFromPitchRollYawDegrees(
			0.f,
			0.f,
			-deltaSeconds* CAMERA_MOVEMENT_SPEED * g_theInput->m_relativeMovement.x );

		m_worldCameraRight.m_transform.Translate( movement );
	}
}

void Game::TestOBJLoader()
{
	std::vector<Vertex_PCUTBN> verts;
	std::vector<uint> indices;
	char const* filename = "Data/Models/Miku.obj";
	mesh_import_options_t meshImportOptionsData;
	LoadOBJToVertexArray( verts, indices, filename, meshImportOptionsData );

	GPUMesh* mesh = new GPUMesh( g_theRenderer );
	mesh->UpdateVertices( verts );
	
	GameObject* miku = new GameObject();
	miku->SetMesh( mesh );
	miku->m_transform.SetPosition( Vec3( 15.f, -0.5f, -10.f ) );
	m_gameObejctsTBN.push_back( miku );
}

void Game::CreateFighterMesh()
{
	std::vector<Vertex_PCUTBN> verts;
	std::vector<uint> indices;
	char const* filename = "Data/Models/vr_controller_vive_1_5.obj";
	mesh_import_options_t meshImportOptionsData;
	meshImportOptionsData.generate_tangents = true;
	LoadOBJToVertexArray( verts, indices, filename, meshImportOptionsData );

	GPUMesh* mesh = new GPUMesh( g_theRenderer );
	mesh->UpdateVertices( verts );

	GameObject* fighter = new GameObject();
	fighter->SetMesh( mesh );
	fighter->m_transform.SetPosition( Vec3( 20.f, -0.5f, -10.f ) );
	m_gameObejctsTBN.push_back( fighter );
}

void Game::CreateCane()
{
	std::vector<Vertex_PCUTBN> verts;
	std::vector<uint> indices;
	char const* filename = "Data/Models/Cane.obj";
	mesh_import_options_t meshImportOptionsData;
	meshImportOptionsData.generate_tangents = true;
	LoadOBJToVertexArray( verts, indices, filename, meshImportOptionsData );

	GPUMesh* mesh = new GPUMesh( g_theRenderer );
	mesh->UpdateVertices( verts );

	GameObject* go = new GameObject();
	go->SetMesh( mesh );
	go->m_transform.SetPosition( Vec3( 30.f, -0.5f, -10.f ) );
	m_gameObejctsTBN.push_back( go );
}

void Game::InitDissolveMaterial()
{
	m_dissolveMaterial = Material::s_definitions["Dissolve"];
	dissolve_data_t dissolve;
	dissolve.burn_amount = 0.9f;
	m_dissolveMaterial->SetData( dissolve );
}

void Game::InitLitMaterial()
{
	m_litMaterial = Material::s_definitions["Lit"];
}

void Game::InitWorldOpaqueMaterial()
{
	m_worldOpaqueMaterial = Material::s_definitions["WorldOpaque"];
}

void Game::InitStoneMaterial()
{
	m_stoneMaterial = Material::s_definitions["Stone"];
}

void Game::InitWorldCameras()
{
	m_worldCameraLeft.SetOutputSize( Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	//m_worldCameraLeft.SetProjectionPerspective( 60.f, -0.1f, -100.f );
	m_worldCameraLeft.SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );
	m_worldCameraLeft.IntialUBO( g_theRenderer );
	m_worldCameraLeft.SetCanClearDepthBuffer( true );

	m_worldCameraRight.SetOutputSize( Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	//m_worldCameraRight.SetProjectionPerspective( 60.f, -0.1f, -100.f );
	m_worldCameraRight.SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );
	m_worldCameraRight.IntialUBO( g_theRenderer );
	m_worldCameraRight.SetCanClearDepthBuffer( true );
}

void Game::InitUICamera()
{
	m_uiCamera.SetOutputSize( Vec2( UI_CAMERA_SIZE_X, UI_CAMERA_SIZE_Y ) );
	m_uiCamera.SetProjectionOrthographic( 100.f, 10.f, -10.f );
	m_uiCamera.SetOrthoView( Vec2::ZERO, Vec2( UI_CAMERA_SIZE_X, UI_CAMERA_SIZE_Y ) );
	m_uiCamera.IntialUBO( g_theRenderer );
	m_uiCamera.SetCanClearDepthBuffer( false );
}

bool Game::InitSoundEffectFiles()
{
	SoundID result;

	result = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
	if( result == MISSING_SOUND_ID )
	{
		return false;
	}

	result = g_theAudio->CreateOrGetSound( "Data/Audio/Teleporter.wav" );
	if( result == MISSING_SOUND_ID )
	{
		return false;
	}

	return true;
}

void Game::CalculateBezierCurve( Vec3& startPoint, Vec3& endPoint, int vertexCount, std::vector<Vec3>& pointList, const Matrix4& controllerModelMatrix )
{
	if( startPoint == Vec3::ZERO )
	{
		Vector4 bezierStartPointVec4 = controllerModelMatrix * Vector4( 0, 0.015f, -0.09f, 1 );
		startPoint = Vec3( bezierStartPointVec4.x, bezierStartPointVec4.y, bezierStartPointVec4.z );
	}
	
	if( endPoint == Vec3::ZERO )
	{
		Vector4 bezierCurveEndPointVec4 = controllerModelMatrix * Vector4( 0, -6.f, -3.f, 1 );
		endPoint = Vec3( bezierCurveEndPointVec4.x, bezierCurveEndPointVec4.y, bezierCurveEndPointVec4.z );
	}

	Vec3 forwardVector = ( endPoint - startPoint ).GetNormalized();
	float fraction = Clamp( GetProjectedLength3D( forwardVector, Vec3( 0, 0, -1.f ) ) / 1.f, 0.f, 1.f );
	float h = Interpolate( 0.f, 2.5f, 1 - fraction );
	Vec3 controlPoint = 0.5f * (startPoint + endPoint) + Vec3( 0, 0, h );

	pointList.clear();
	pointList.reserve( (size_t)vertexCount + 1 );
	for( float t = 0.f; t <= 1.f; t += 1.f / vertexCount )
	{
		Vec3 tangentLineVert1 = Interpolate( startPoint, controlPoint, t );
		Vec3 tangentLineVert2 = Interpolate( controlPoint, endPoint, t );
		Vec3 bezierPoint = Interpolate( tangentLineVert1, tangentLineVert2, t );
		pointList.push_back( bezierPoint );
	}
}

Vec3 Game::LinePlaneIntersection( Vec3 planePoint, Vec3 planeNormal, Vec3 linePoint, Vec3 lineDirection )
{
	lineDirection = lineDirection.GetNormalized();
	if( DotProduct( planeNormal, lineDirection ) == 0 )
	{
		return Vec3::ZERO;
	}

	double t = ( DotProduct( planeNormal, planePoint ) - DotProduct( planeNormal, linePoint ) ) / DotProduct( planeNormal, lineDirection );
	return linePoint + lineDirection * (float)t;
}

Mat44 Game::ConvertMatrix4ToMat44( Matrix4 other )
{
	Mat44 mat = Mat44( &other.get()[0] );
	return mat;
}


void Game::UpdateCursorPosition()
{
	Vec2 mousePos = g_theInput->GetMouseNormalizedClientPos();
	m_cursorPosition = m_uiCamera.ClientToWorldPosition( mousePos );
}

void Game::AdjustAmbientIntensity()
{
	if( g_theConsole->IsOpen() ) {
		return;
	}

	if( g_theInput->IsKeyPressed( KEY_9 ) ) 
	{
		m_ambientIntensity -= AMBIENT_ADD_SUBSTRACT_AMOUNT;
		m_ambientIntensity = Clamp( m_ambientIntensity, 0.f, 1.f );
	}
	else if( g_theInput->IsKeyPressed( KEY_0 ) ) 
	{
		m_ambientIntensity += AMBIENT_ADD_SUBSTRACT_AMOUNT;
		m_ambientIntensity = Clamp( m_ambientIntensity, 0.f, 1.f );
	}
}

void Game::ToggleLightAttenuationMode()
{
	// toggle attenuation using `T`
	if( g_theInput->WasKeyJustPressed( KEY_T ) && !g_theConsole->IsOpen() ) {
		eAttenuationMode currentLightAttMode = g_theRenderer->CovertAttToEnumAttMode( m_lightMaster.lightConstants.lights[m_lightMaster.lightIdx].attenuation );
		int attIdx = (int)currentLightAttMode;
	
		attIdx++;
		if( attIdx >= (int)eAttenuationMode::NUM_ATTENUATION_MODES ) {
			attIdx = 0;
		}

		Vec3 att = g_theRenderer->GetAttenuationFromAttMode( (eAttenuationMode)attIdx );
		m_lightMaster.lightConstants.lights[m_lightMaster.lightIdx].attenuation = att;
	}	
}

void Game::UpdateLightKeyFunctions()
{
	// Start Position Y, decrease by 0.03f every line of string.
	float startPositionY = 0.43f;

	// debug text 
	std::string text = "[ F5 ] Position at Origin";
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = "[ F6 ] Position at Camera";
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = "[ F7 ] Follow Camera";
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = "[ F8 ] Follow Animated Path";
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = Stringf("[    ] Gamma: %.2f", m_gamma );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = Stringf( "[9, 0] Ambient Intensity: %.2f", m_ambientIntensity );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = Stringf( "[-, +] Change Intensity: %.2f", m_lightMaster.lightConstants.lights[m_lightMaster.lightIdx].intensity );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	// print debug text showing the current attenuation mode
	eAttenuationMode currentLightAttMode = g_theRenderer->CovertAttToEnumAttMode( m_lightMaster.lightConstants.lights[m_lightMaster.lightIdx].attenuation );
	text = "[ T ]Attenuation mode: " + g_theRenderer->GetAttenuationAsString( currentLightAttMode );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = Stringf( "[{, }] Specular Factor: %.2f", g_theRenderer->m_specularFactor );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = Stringf( "[;, '] Specular Power: %.2f", g_theRenderer->m_specularPower );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = Stringf( "[N, M] Current Light: lights[%d]", m_lightMaster.lightIdx );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = Stringf( "[O, P] Dissolve amount: %.2f", 1.f - g_theRenderer->m_materialData.burn_amount );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = Stringf( "[1, 2] Depth: %.2f", g_theRenderer->m_materialData.depth );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = Stringf( "[U, I] GrayScale: %.2f", g_theRenderer->m_imageEffectData.grayScale );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, startPositionY, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
	startPositionY -= SCREEN_DEBUG_TEXT_LINE_HEIGHT;

	text = "Current Light behavior: " + GetLightBehaviorAsString( m_lightMaster.lightBehaviors[m_lightMaster.lightIdx] );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.001f, 0.001f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::CYAN, Rgba8::WHITE, 0.f, text.c_str() );

	// early out
	if( g_theConsole->IsOpen() ) {
		return;
	}

	if( g_theInput->WasKeyJustPressed( KEY_F5 ) ) {
		//m_pointLight.position = Vec3::ZERO;
		m_lightMaster.lightConstants.lights[m_lightMaster.lightIdx].position = Vec3::ZERO;
	}

	if( g_theInput->WasKeyJustPressed( KEY_F6 ) ) {
		//m_pointLight.position = m_worldCamera.m_transform.m_position;
		m_lightMaster.lightConstants.lights[m_lightMaster.lightIdx].position = m_worldCameraLeft.m_transform.m_position;;
	}

	if( g_theInput->WasKeyJustPressed( KEY_F7 ) ) {
		//m_isPointLightFollowCamera = !m_isPointLightFollowCamera;
		if( m_lightMaster.lightBehaviors[m_lightMaster.lightIdx] == eLightBehavior::STAY ) {
			m_lightMaster.lightBehaviors[m_lightMaster.lightIdx] = eLightBehavior::FOLLOW_CAMERA;
		}
		else {
			m_lightMaster.lightBehaviors[m_lightMaster.lightIdx] = eLightBehavior::STAY;
		}	
	}


	if( g_theInput->IsKeyPressed( KEY_MINUS ) ) {
		float& intensity = m_lightMaster.lightConstants.lights[m_lightMaster.lightIdx].intensity;
		intensity -= 0.05f;
		if( intensity < 0.f ) {
			intensity = 0.f;
		}
	}

	if( g_theInput->IsKeyPressed( KEY_PLUS ) ) {
		m_lightMaster.lightConstants.lights[m_lightMaster.lightIdx].intensity += 0.05f;
	}


	if( g_theInput->IsKeyPressed( KEY_LBRACKET ) ) {
		g_theRenderer->m_specularFactor -= 0.02f;
		if( g_theRenderer->m_specularFactor < 0.f ) {
			g_theRenderer->m_specularFactor = 0.f;
		}
	}

	if( g_theInput->IsKeyPressed( KEY_RBRACKET ) ) {
		g_theRenderer->m_specularFactor += 0.02f;
		if( g_theRenderer->m_specularFactor > 1.f ) {
			g_theRenderer->m_specularFactor = 1.f;
		}
	}

	if( g_theInput->IsKeyPressed( KEY_COLON ) ) {
		g_theRenderer->m_specularPower -= 0.1f;
		if( g_theRenderer->m_specularPower < 1.f ) {
			g_theRenderer->m_specularPower = 1.f;
		}
	}

	if( g_theInput->IsKeyPressed( KEY_APOSTROPHE ) ) {
		g_theRenderer->m_specularPower += 0.1f;
	}

	if( g_theInput->WasKeyJustPressed( KEY_N ) ) {
		m_lightMaster.lightIdx--;
		if( m_lightMaster.lightIdx < 0 ) {
			m_lightMaster.lightIdx = MAX_NUM_LIGHTS - 1;
		}
	}
	if( g_theInput->WasKeyJustPressed( KEY_M ) ) {
		m_lightMaster.lightIdx++;
		if( m_lightMaster.lightIdx >= MAX_NUM_LIGHTS ) {
			m_lightMaster.lightIdx = 0;
		}
	}

	if( g_theInput->IsKeyPressed( 'U' ) )
	{
		image_effect_data_t& imageEffectData = g_theRenderer->m_imageEffectData;
		imageEffectData.grayScale -= 0.01f;
		imageEffectData.grayScale = Clamp( imageEffectData.grayScale, 0.f, 1.f );
	}

	if( g_theInput->IsKeyPressed( 'I' ) )
	{
		image_effect_data_t& imageEffectData = g_theRenderer->m_imageEffectData;
		imageEffectData.grayScale += 0.01f;
		imageEffectData.grayScale = Clamp( imageEffectData.grayScale, 0.f, 1.f );
	}


}

void Game::SetLightType( light_t& light, eLightType lightType )
{
	switch( lightType ) {
	case eLightType::DIRECTION_LIGHT: light.direction_factor = 1.f; break;
	case eLightType::POINT_LIGHT:
	case eLightType::SPOT_LIGHT:
		light.direction_factor = 0.0f; break;	// point light and spot light both set direction factor to 0.f
	}
}

void Game::SetLightType( uint idx, eLightType lightType )
{
	ASSERT_OR_DIE( (idx >= 0 && idx < MAX_NUM_LIGHTS), "Error index is out of range." );

	SetLightType( m_lightMaster.lightConstants.lights[idx], lightType );
}

std::string Game::GetLightBehaviorAsString( eLightBehavior behavior )
{
	switch( behavior ) {
	case eLightBehavior::FOLLOW_CAMERA: return "Following Camera"; break;
	case eLightBehavior::STAY:			return "Idle";			break;
	default: ERROR_AND_DIE("Unknown behavior type.");			break;
	}
}

//-------------------------------------------------------------------------------------------------------------
void Game::UpdateMaterialConstants()
{
	material_data_t& materialData = g_theRenderer->m_materialData;
	materialData.color = ConvertRgba8ToVec3( Rgba8::GREEN );
	materialData.fresnel_power = 1.f;

	if( g_theInput->IsKeyPressed( 'P' ) ) {
		materialData.burn_amount -= 0.01f;
		if( materialData.burn_amount < 0.f ) {
			materialData.burn_amount = 0.f;
		}
	}
	if( g_theInput->IsKeyPressed( 'O' ) ) {
		materialData.burn_amount += 0.01f;
		if( materialData.burn_amount > 1.f ) {
			materialData.burn_amount = 1.f;
		}
	}

	if( g_theInput->IsKeyPressed( KEY_1 ) ) {
		materialData.depth -= 0.005f;
	}
	if( g_theInput->IsKeyPressed( KEY_2 ) ) {
		materialData.depth += 0.005f;
	}	
	materialData.depth = Clamp( materialData.depth, 0.f, 0.25f );
}

//-------------------------------------------------------------------------------------------------------------
void Game::AddCameraInfoScreenText()
{
	if( !m_isDebugRenderingActive ) 
	{
		return;
	}

	// Print World Camera's Transform Pitch Row Yaw Degrees
	std::string text = Stringf( "LEFT Camera Yaw=%.1f  Pitch=%.1f  Roll=%.1f   xyz=(%.2f, %.2f, %.2f)",
		m_worldCameraLeft.m_transform.m_orientationYaw,
		m_worldCameraLeft.m_transform.m_orientationPitch,
		m_worldCameraLeft.m_transform.m_orientationRoll,
		m_worldCameraLeft.m_transform.m_position.x,
		m_worldCameraLeft.m_transform.m_position.y,
		m_worldCameraLeft.m_transform.m_position.z );

	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.01f, 0.95f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::YELLOW, Rgba8::YELLOW, 0.f, text.c_str() );

	text = Stringf( "RIGHT Camera Yaw=%.1f  Pitch=%.1f  Roll=%.1f   xyz=(%.2f, %.2f, %.2f)",
		m_worldCameraRight.m_transform.m_orientationYaw,
		m_worldCameraRight.m_transform.m_orientationPitch,
		m_worldCameraRight.m_transform.m_orientationRoll,
		m_worldCameraRight.m_transform.m_position.x,
		m_worldCameraRight.m_transform.m_position.y,
		m_worldCameraRight.m_transform.m_position.z );

	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.01f, 0.83f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::YELLOW, Rgba8::YELLOW, 0.f, text.c_str() );

	Mat44 cameraMatrix = m_worldCameraLeft.m_transform.ToMatrix();
	Vec3 iBasis = cameraMatrix.GetIBasis3D();
	Vec3 jBasis = cameraMatrix.GetJBasis3D();
	Vec3 kBasis = cameraMatrix.GetKBasis3D();
	text = Stringf( "iBasis (forward, +x world-east when identity): (%.2f, %.2f, %.2f)", iBasis.x, iBasis.y, iBasis.z );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.01f, 0.92f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::RED, Rgba8::RED, 0.f, text.c_str() );
	text = Stringf( "jBasis (left, +y world-east when identity): (%.2f, %.2f, %.2f)", jBasis.x, jBasis.y, jBasis.z );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.01f, 0.89f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::GREEN, Rgba8::GREEN, 0.f, text.c_str() );
	text = Stringf( "kBasis (up, +z world-east when identity): (%.2f, %.2f, %.2f)", kBasis.x, kBasis.y, kBasis.z );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.01f, 0.86f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::BLUE, Rgba8::BLUE, 0.f, text.c_str() );

	text = Stringf( "%.2f, %.2f, %.2f", m_teleportDestination.x, m_teleportDestination.y, m_teleportDestination.z );
	g_theDebugRenderSystem->DebugAddScreenText( Vec4( 0.01f, 0.5f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::YELLOW, Rgba8::YELLOW, 0.f, text.c_str() );
}

void Game::UpdateLightBehaviors()
{
	//if( m_isDebugRenderingActive )
	//{
	//	for( int i = 0; i < 4; ++i )
	//	{
	//		if( m_lightMaster.lightBehaviors[0] == eLightBehavior::STAY ) {
	//			AABB3 bounds( m_lightMaster.lightConstants.lights[i].position - Vec3( 0.1f, 0.1f, 0.1f ), m_lightMaster.lightConstants.lights[0].position + Vec3( 0.1f, 0.1f, 0.1f ) );
	//			g_theDebugRenderSystem->DebugAddWorldWireBounds( bounds, m_lightMaster.lightConstants.lights[i].color );
	//		}
	//	}
	//}
}

//-------------------------------------------------------------------------------------------------------------
void Game::DectectVRControllerRaycast()
{
	if( !g_theLighthouse || !g_theLighthouse->IsValid() )
		return;

	if( m_teleportingState == eVRControllerTeleportingState::INVALID )
		return;


	for( vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice )
	{
		if( !g_theLighthouse->m_pHMD->IsTrackedDeviceConnected( unTrackedDevice ) )
			continue;

		if( g_theLighthouse->m_pHMD->GetTrackedDeviceClass( unTrackedDevice ) != vr::TrackedDeviceClass_Controller )
			continue;

		if( !g_theLighthouse->m_rTrackedDevicePose[unTrackedDevice].bPoseIsValid )
			continue;

		vr::ETrackedControllerRole role = g_theLighthouse->m_pHMD->GetControllerRoleForTrackedDeviceIndex( unTrackedDevice );
		if( !( (role == vr::TrackedControllerRole_LeftHand && m_teleportingState == eVRControllerTeleportingState::LEFT_HAND_TELEPORTING) ||
			   (role == vr::TrackedControllerRole_RightHand && m_teleportingState == eVRControllerTeleportingState::RIGHT_HAND_TELEPORTING) ) )
			continue;

		Matrix4 controllerModelMatrix = g_theLighthouse->GetControllerModelMatrix( unTrackedDevice );
		TileMap* tileMap = dynamic_cast< TileMap* >( m_theWorld->m_currentMap );
		//Vector4 start = controllerModelMatrix * Vector4( 0, 0, -0.02f, 1 );
		//Vector4 end = controllerModelMatrix * Vector4( 0, 0, -20.f, 1 );
		//Vec3 startPos = Vec3( start.x, start.y, start.z );
		//Vec3 endPos = Vec3( end.x, end.y, end.z );

		////const Rgba8& colorForwardVector =  Rgba8::CYAN;
		////float thickness = 0.0025f;
		////g_theDebugRenderSystem->DebugAddWorldLine( startPos, colorForwardVector, endPos, colorForwardVector, thickness );

		//// ----- Raycast -----
		//Vec3 forwardVector = ( endPos - startPos ).GetNormalized();
		//Vec2 forwardVectorForCeilingAndFloor = Vec2( sqrtf( forwardVector.x * forwardVector.x + forwardVector.y * forwardVector.y ), forwardVector.z ).GetNormalized();
		//// Hack to call member function in TileMap

		////float xy = sqrtf( startPos.x * startPos.x + startPos.y * startPos.y );
		//constexpr float maxDistance = 20.f;
		//RaycastResult WallTestResult = tileMap->Raycast( Vec2( startPos.x, startPos.y ), Vec2( forwardVector.x, forwardVector.y ), maxDistance );
		//RaycastResult ZTestResult = tileMap->RaycastAgainstCeilingAndFloor( Vec2( 0.f, startPos.z ), forwardVectorForCeilingAndFloor, maxDistance, CEILING_HEIGHT );
		//bool isRaycastResultValid = ( !WallTestResult.m_didImpact && ZTestResult.m_didImpact ) || ( WallTestResult.m_didImpact && ZTestResult.m_didImpact && WallTestResult.m_impactFraction > ZTestResult.m_impactFraction );
		////if( ZTestResult.m_didImpact )
		//if( isRaycastResultValid )
		//{
		//	Vec3 impactPoint = startPos + ( forwardVector * ZTestResult.m_impactFraction * ZTestResult.m_maxDistance );
		//	IntVec2 tileCoord = tileMap->GetTileCoordsForWorldPosition( Vec2( impactPoint.x, impactPoint.y ) );

		//	bool isLocationValidToTeleport = !tileMap->IsTileSolid( tileCoord ) && impactPoint.z <= 0.1f;
		//	Rgba8 pointColor = ( isLocationValidToTeleport ) ? Rgba8::GREEN : Rgba8::RED;
		//	g_theDebugRenderSystem->DebugAddWorldPoint( impactPoint, 0.05f, pointColor );
		//	
		//	if( isLocationValidToTeleport )
		//	{
		//		m_teleportDestination = Vec3( impactPoint.x, impactPoint.y, 0.f ); //#ToDo: Apply the correct z pos
		//	}
		//}
		//if( WallTestResult.m_didImpact && WallTestResult.m_impactFraction < ZTestResult.m_impactFraction )
		//{
		//	Vec3 impactPoint = startPos + forwardVector * WallTestResult.m_impactFraction * maxDistance;
		//	g_theDebugRenderSystem->DebugAddWorldPoint( impactPoint, 0.05f, Rgba8::RED );
		//}

		// Bezier Curve
		Vec3 bezierStartPoint, bezierEndPoint;
		const int vertexCount = 12;
		std::vector< Vec3 > pointList;
		CalculateBezierCurve( bezierStartPoint, bezierEndPoint, vertexCount, pointList, controllerModelMatrix );
		
		Rgba8 bezierCurveColor = Rgba8::RED;

		for( int pointIdx = 0; pointIdx < pointList.size() - 1; pointIdx++ )
		{
			Vec3 lineDir3D = ( pointList[pointIdx + 1] - pointList[pointIdx] ).GetNormalized();
			// Raycast against walls
			RaycastResult result = tileMap->Raycast( Vec2( pointList[pointIdx].x, pointList[pointIdx].y ), Vec2( lineDir3D.x, lineDir3D.y ).GetNormalized(), GetDistance3D( pointList[pointIdx], pointList[pointIdx + 1] ) );
			if( result.m_didImpact ) {
				Vec3 invalidPointOnWall = pointList[pointIdx] + lineDir3D * result.m_impactFraction * result.m_maxDistance;
				g_theDebugRenderSystem->DebugAddWorldPoint( invalidPointOnWall, 0.05f, Rgba8::RED );
				bezierEndPoint = invalidPointOnWall;
				break;
			}

			// Intersection with floor
			Vec3 intersectionPoint = LinePlaneIntersection( Vec3::ZERO, Vec3( 0, 0, 1.f ), pointList[pointIdx], lineDir3D );
			if( GetDistanceSquared3D( pointList[pointIdx], pointList[pointIdx + 1] ) >= GetDistanceSquared3D( pointList[pointIdx], intersectionPoint ) && intersectionPoint != Vec3::ZERO )
			{
				g_theDebugRenderSystem->DebugAddWorldPoint( intersectionPoint, 0.05f, Rgba8::GREEN );
				bezierEndPoint = intersectionPoint;
				m_teleportDestination = intersectionPoint;
				bezierCurveColor = Rgba8::GREEN;
				break;
			}

			//g_theDebugRenderSystem->DebugAddWorldLine( pointList[pointIdx], Rgba8::CYAN, pointList[pointIdx + 1], Rgba8::CYAN, 0.005f );
		}

		CalculateBezierCurve( bezierStartPoint, bezierEndPoint, vertexCount, pointList, controllerModelMatrix );

		// Draw Lines
		for( int pointIdx = 0; pointIdx < pointList.size() - 1; pointIdx++ )
		{
			g_theDebugRenderSystem->DebugAddWorldLine( pointList[pointIdx], bezierCurveColor, pointList[(size_t)pointIdx + 1], bezierCurveColor, 0.005f );
		}
	}
}

void Game::UpdateTeleportingState( std::string const& controllerRole )
{
	if( controllerRole == "LeftHand" )
	{
		m_teleportingState = eVRControllerTeleportingState::LEFT_HAND_TELEPORTING;
	}
	else if(  controllerRole == "RightHand" )
	{
		m_teleportingState = eVRControllerTeleportingState::RIGHT_HAND_TELEPORTING;
	}
	else 
	{
		m_teleportingState = eVRControllerTeleportingState::INVALID;
	}
}

void Game::TeleportPlayer( std::string const& controllerRole )
{
	switch( m_teleportingState )
	{
	case eVRControllerTeleportingState::LEFT_HAND_TELEPORTING:
		{
			if( controllerRole == "LeftHand" )
			{
				m_theWorld->EnterMap( m_theWorld->m_currentMap, m_teleportDestination, GetPlayer()->m_yawDegrees );
				m_teleportingState = eVRControllerTeleportingState::INVALID;
			}
		}
		break;

	case eVRControllerTeleportingState::RIGHT_HAND_TELEPORTING:
		{
			if( controllerRole == "RightHand" )
			{
				m_theWorld->EnterMap( m_theWorld->m_currentMap, m_teleportDestination, GetPlayer()->m_yawDegrees );
				m_teleportingState = eVRControllerTeleportingState::INVALID;
			}
		}
		break;

	default:
		//g_theConsole->Printf( "%s - Failed to Teleport Player: Invalid Controller Role", __FUNCTION__ );
		m_teleportingState = eVRControllerTeleportingState::INVALID;
		break;
	}
}

//-------------------------------------------------------------------------------------------------------------
void Game::Fire( std::string const& controllerRole )
{
	if( !g_theLighthouse || !g_theLighthouse->IsValid() )
		return;

	// Play Audio


	for( vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice )
	{
		if( !g_theLighthouse->m_pHMD->IsTrackedDeviceConnected( unTrackedDevice ) )
			continue;

		if( g_theLighthouse->m_pHMD->GetTrackedDeviceClass( unTrackedDevice ) != vr::TrackedDeviceClass_Controller )
			continue;

		if( !g_theLighthouse->m_rTrackedDevicePose[unTrackedDevice].bPoseIsValid )
			continue;

		vr::ETrackedControllerRole role = g_theLighthouse->m_pHMD->GetControllerRoleForTrackedDeviceIndex( unTrackedDevice );
		bool isLeftController = ( role == vr::TrackedControllerRole_LeftHand ) && ( controllerRole == "LeftHand" );
		bool isRightController = ( role == vr::TrackedControllerRole_RightHand ) && ( controllerRole == "RightHand" );
		if( !isLeftController && !isRightController )
			continue;

		Matrix4 controllerModelMatrix = g_theLighthouse->GetControllerModelMatrix( unTrackedDevice );
		Vector4 start = controllerModelMatrix * Vector4( 0, 0, -0.02f, 1 );
		Vector4 end = controllerModelMatrix * Vector4( 0, 0, -39.f, 1 );
		Vec3 startPos = Vec3( start.x, start.y, start.z );
		Vec3 endPos = Vec3( end.x, end.y, end.z );
		Vec3 forwardVector = ( endPos - startPos ).GetNormalized();

		TileMap* tileMap = dynamic_cast<TileMap*>(m_theWorld->m_currentMap);
		if( !tileMap ) {
			g_theConsole->Error( "%s - Failed to get TileMap!", __FUNCTION__ );
			return;
		}
		
		//-------------------------------------------------------------------------------------------------------------
		Projectile* projectile = dynamic_cast<Projectile*>( tileMap->SpawnNewEntityOfType( "Plasma Bolt" ) );
		if( projectile ) {
			Vec3 spawnPoint = startPos + forwardVector * 0.1f;
			projectile->m_position = Vec2( spawnPoint.x, spawnPoint.y );
			projectile->m_height = spawnPoint.z;
			projectile->SetVelocity( forwardVector * BULLET_SPEED );
			projectile->SetFaction( Faction::GOOD );

			// Calculate jBasis
	/*		Vector4 rightPoint = controllerModelMatrix * Vector4( 1, 0, 0, 1 );
			Vec3 rightPos = Vec3( rightPoint.x, rightPoint.y, rightPoint.z );
			Vec3 jBasis = (rightPos - startPos).GetNormalized();
			projectile->m_jBasis = jBasis;*/
		}

		//-------------------------------------------------------------------------------------------------------------
		float maxImpactDistance = 5.f;
		RaycastResult result = tileMap->RaycastAgainstEntities( startPos, forwardVector, maxImpactDistance );

		if( result.m_didImpact )
		{
			Entity& e = *result.m_impactEntity;
			if( !e.IsProjectile() && !e.IsDead() )
			{
				// ---- Debug Render -----
				//Vec3 min = Vec3( e.m_position.x - e.m_radius, e.m_position.y - e.m_radius, 0.f );
				//Vec3 max = Vec3( e.m_position.x + e.m_radius, e.m_position.y + e.m_radius, e.m_height );
				//AABB3 aabb3 = AABB3( min, max );
				//g_theDebugRenderSystem->DebugAddWorldWireBounds( aabb3, Rgba8::WHITE, 1.f );
				e.m_state = AIState::ATTACK;

				// -----Apply Damage -----
				Vec3 impactPoint = startPos + forwardVector * result.m_impactFraction;
				if( impactPoint.z > 1.25f ) {
					e.TakeDamage( 50 );
					g_theDebugRenderSystem->DebugAddWorldPoint( impactPoint, 0.015f, Rgba8::RED, 0.5f );
				}
				else {
					e.TakeDamage( 10 );
					g_theDebugRenderSystem->DebugAddWorldPoint( impactPoint, 0.015f, Rgba8::WHITE, 0.5f );
				}

				// Cast to Actor( Pinky )
				Actor* pinky = dynamic_cast<Actor*>(result.m_impactEntity);
				if( pinky )
				{
					pinky->m_hurtTimer.SetSeconds( 0.075f );
					pinky->SetAIState( AIState::HURT );
				}
			}		
		}
	}
}

void Game::PlayerRequestMoveForward()
{
	Entity* player = GetPlayer();
	if( !player ) {
		return;
	}

	if( m_locomoiton == eLocomotion::CONTINUOUS )
	{
		m_isTouchpadTopButtonDown = true;
		m_isTouchpadBottomButtonDown = false;
	}
	else 
	{	// Move some distance forward
		TileMap* tileMap = dynamic_cast<TileMap*>(m_theWorld->m_currentMap);
		if( tileMap )
		{
			Vec3 hmdEulerAngles = g_theLighthouse->GetHMDEulerAngles();
			hmdEulerAngles = Vec3( ConvertRadiansToDegrees( hmdEulerAngles.x ),
				ConvertRadiansToDegrees( hmdEulerAngles.x ),
				ConvertRadiansToDegrees( hmdEulerAngles.x ) );

			float hmdYawDiff = hmdEulerAngles.x - g_theLighthouse->m_previousHMDYawDegrees;
			Vec2 forwardDir = Vec2( 1.f, 0 ).GetRotatedDegrees( player->m_yawDegrees + hmdYawDiff ).GetNormalized();

			//Vec2 pointToCheck = player->m_position + player->GetForwardVector() * 0.25f;
			Vec2 pointToCheck = player->m_position + forwardDir * 0.25f;
			IntVec2 tileCoord = tileMap->GetTileCoordsForWorldPosition( pointToCheck );
			bool isLocationValidToTeleport = !tileMap->IsTileSolid( tileCoord );
			if( isLocationValidToTeleport )
			{
				m_theWorld->EnterMap( m_theWorld->m_currentMap, Vec3( pointToCheck, 0 ), player->m_yawDegrees );
			}
		}
	}
}

void Game::PlayerRequestMoveBackward()
{
	Entity* player = GetPlayer();
	if( !player ) {
		return;
	}

	if( m_locomoiton == eLocomotion::CONTINUOUS )
	{
		m_isTouchpadBottomButtonDown = true;
		m_isTouchpadTopButtonDown = false;
	}
	else
	{
		Vec3 hmdEulerAngles = g_theLighthouse->GetHMDEulerAngles();
		hmdEulerAngles = Vec3( ConvertRadiansToDegrees( hmdEulerAngles.x ),
			ConvertRadiansToDegrees( hmdEulerAngles.x ),
			ConvertRadiansToDegrees( hmdEulerAngles.x ) );

		float hmdYawDiff = hmdEulerAngles.x - g_theLighthouse->m_previousHMDYawDegrees;
		Vec2 forwardDir = Vec2( 1.f, 0 ).GetRotatedDegrees( player->m_yawDegrees + hmdYawDiff ).GetNormalized();
		Vec2 pointToCheck = player->m_position - forwardDir * 0.25f;
		TileMap* tileMap = dynamic_cast<TileMap*>(m_theWorld->m_currentMap);
		IntVec2 tileCoord = tileMap->GetTileCoordsForWorldPosition( Vec2( pointToCheck.x, pointToCheck.y ) );

		bool isLocationValidToTeleport = !tileMap->IsTileSolid( tileCoord );
		if( isLocationValidToTeleport )
		{
			m_theWorld->EnterMap( m_theWorld->m_currentMap, Vec3( pointToCheck, 0 ), player->m_yawDegrees );
		}
	}
}

Entity* Game::GetPlayer()
{
	Entity* player = m_theWorld->m_currentMap->m_allEntities[0];
	if( player && player->IsPlayer() )
	{
		return player;
	}

	return nullptr;
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( light_set_ambient_color, " " )
{
	UNUSED(args);
	for( int i = 0; i < (int)g_theEventSystem->m_eventSubsrciptions.size(); ++i )
	{
		if( g_theEventSystem->m_eventSubsrciptions[i]->m_eventName == "light_set_ambient_color" )
		{
			NamedProperties namedProperties = g_theEventSystem->m_eventSubsrciptions[i]->m_input;
			Rgba8 color = namedProperties.GetValue( "color", Rgba8::WHITE );
			g_theGame->m_ambientLightColor = color;
			g_theConsole->PrintString( Rgba8::CYAN, "light ambient color has updated" );
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( light_set_color, " " )
{
	UNUSED(args);
	for( int i = 0; i < (int)g_theEventSystem->m_eventSubsrciptions.size(); ++i )
	{
		if( g_theEventSystem->m_eventSubsrciptions[i]->m_eventName == "light_set_color" )
		{
			NamedProperties namedProperties =  g_theEventSystem->m_eventSubsrciptions[i]->m_input;
			Rgba8 color = namedProperties.GetValue( "color", Rgba8::WHITE );
			g_theGame->m_pointLight.color.x = (float)(color.r / 255);
			g_theGame->m_pointLight.color.y = (float)(color.g / 255);
			g_theGame->m_pointLight.color.z = (float)(color.b / 255);
			g_theConsole->PrintString( Rgba8::CYAN, "light color has updated" );
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( BEGIN_TELEPORT, "role" )
{
	std::string roleStr = args.GetValue( "role", std::string() );
	g_theGame->UpdateTeleportingState( roleStr );
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( END_TELEPORT, "role" )
{
	std::string roleStr = args.GetValue( "role", std::string() );
	g_theGame->TeleportPlayer( roleStr );
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( FIRE, "role" )
{
	std::string roleStr = args.GetValue( "role", std::string() );
	//g_theConsole->Printf( "COMMAND: %s", roleStr.c_str() );

	// Call Fire Function in game
	 g_theGame->Fire( roleStr );
}