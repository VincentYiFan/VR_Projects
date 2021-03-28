#pragma once
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include <vector>

//---------------------------------------------------------------------------------------------------------
class Window;

//---------------------------------------------------------------------------------------------------------
enum GameState
{
	GAME_STATE_NONE = -1,

	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,

	NUM_GAME_STATE
};
//---------------------------------------------------------------------------------------------------------
enum class eBillboardMode : unsigned int
{
	BILLBOARD_MODE_CAMERA_FACING_XY,
	BILLBOARD_MODE_CAMERA_OPPOSING_XY,
	BILLBOARD_MODE_CAMERA_FACING_XYZ,
	BILLBOARD_MODE_CAMERA_OPPOSING_XYZ,	// Screen-aligned
	NUM_BILLBOARD_MODES
};
eBillboardMode	GetBillboardModeFromName( std::string const& modeName );
void GetBillboardQuad( Vec3& out_BL, Vec3& out_BR, Vec3& out_TR, Vec3& out_TL, Camera const& cam, Vec3 center, Vec2 size, eBillboardMode mode );


//---------------------------------------------------------------------------------------------------------
constexpr float CLIENT_ASPECT = 1.7778f; // We are requesting a 16:9 aspect (square) window area
constexpr float UI_CAMERA_SIZE_X = 200.f;
constexpr float UI_CAMERA_SIZE_Y = 100.f;
constexpr int MAP_SIZE_X = 20;
constexpr int MAP_SIZE_Y = 30;
constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 2.f;
constexpr float CAMERA_MOVE_SPEED = 25.f;
constexpr float CAMERA_DEFAULT_HEIGHT = 100.f;
constexpr float POINT_RADIUS = 0.5f;
constexpr float GRAVITY_ADD_SUBSTRACT_AMOUNT = 0.0075f;
constexpr int	NUM_SPHERE = 8;
constexpr float AMBIENT_ADD_SUBSTRACT_AMOUNT = 0.01f; 
constexpr float CAMERA_MOVEMENT_SPEED = 3.0f;
constexpr float SCREEN_DEBUG_TEXT_LINE_HEIGHT = 0.03f;
constexpr float COS_45_DEGREES = 0.52532198881f;

constexpr float CEILING_HEIGHT = 3.f;

// Pinky
constexpr float PINKY_ATTACK_COOLDOWN = 2.f;
constexpr float PINKY_MAX_VISION_RANGE = 4.f;

// Ranged Enemy
constexpr float RANGED_ENEMY_ATTACK_CD = 2.f;

// WEAPON
constexpr float CONTROLLER_PULSE_DURATION = 0.1f;
constexpr float BULLET_HEIGHT = 0.02f;
constexpr float BULLET_SPEED = 15.f;

void AppendVertsForAABB2D( std::vector<Vertex_PCU>& verts, const AABB2& localBounds, const Rgba8 color );
void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, const AABB2& localBounds, const Rgba8 color, const Vec2& uvAtMins, const Vec2& uvAtMaxs );
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& verts, const OBB2& localBounds, const Rgba8 color );


extern App* g_theApp;
extern Game* g_theGame;
extern RenderContext* g_theRenderer;
extern RandomNumberGenerator* g_rng;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern GameState	g_gameState;
extern BitmapFont*  g_theFont;
extern Window* g_theWindow;
extern DebugRender* g_theDebugRenderSystem;
extern LighthouseTracking* g_theLighthouse;

extern eBillboardMode g_debugBillboardMode;


