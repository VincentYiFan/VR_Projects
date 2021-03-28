#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Input/InputSystem.hpp"
//#include "Engine/Core/JobSystem.hpp"

typedef unsigned int uint;

#define UNUSED(x) (void)(x);
#define STATIC
//#define MSAA_ENABLED

//-------------------------------------------------------------------------------------------------------------
class NetworkSystem;

//-------------------------------------------------------------------------------------------------------------
extern NamedStrings		g_gameConfigBlackboard;
extern DevConsole*		g_theConsole;
extern EventSystem*		g_theEventSystem;
extern InputSystem*		g_theInput;
extern NetworkSystem*	g_theNetwork;
//extern JobSystem*	g_theJobSystem;
//-------------------------------------------------------------------------------------------------------------

const Vec2 ALIGN_BOTTOM_LEFT	= Vec2( 0.0f, 0.0f );
const Vec2 ALIGN_CENTERED		= Vec2( 0.5f, 0.5f );
const Vec2 ALIGN_TOP_RIGHT		= Vec2( 1.0f, 1.0f );
const Vec2 ALIGN_BOTTOM_CENTER  = Vec2( 0.5f, 0.0f );
const Vec2 ALIGN_BOTTOM_RIGHT   = Vec2( 1.0f, 0.0f );
const Vec2 ALIGN_MIDDLE_LEFT	= Vec2( 0.0f, 0.5f );
const Vec2 ALIGN_MIDDLE_RIGHT	= Vec2( 1.0f, 0.5f );
const Vec2 ALIGN_TOP_LEFT		= Vec2( 0.0f, 1.0f );
const Vec2 ALIGN_TOP_CENTER		= Vec2( 0.5f, 1.0f );

const Rgba8 CONSOLE_COLOR_ERROR_MAJOR = Rgba8::RED;

constexpr float FOG_FAR_DIST = 75.f;

// JobSystem 
constexpr int	NUM_WORKER_THREADS = 8;
constexpr int	NUM_JOBS = 100;


