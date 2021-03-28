#include "Game/App.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Network/NetworkSystem.hpp"
#include "Game/LighthouseTracking.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


InputSystem*			g_theInput			= nullptr;
AudioSystem*			g_theAudio			= nullptr;
RenderContext*			g_theRenderer		= nullptr;
DevConsole*				g_theConsole		= nullptr;
JobSystem*				g_theJobSystem		= nullptr;
NetworkSystem*			g_theNetwork		= nullptr;
LighthouseTracking*		g_theLighthouse		= nullptr;
GameState				g_gameState			= GAME_STATE_PLAYING;  // Default Game State 

//-------------------------------------------------------------------------------------------------------------
#pragma comment(lib, "Ws2_32.lib")

App::App()
{
}

App::~App()
{
}

void App::Startup( )
{
	Clock::SystemStartup();

	g_theGame =  new Game();
	g_theInput = new InputSystem();
	//g_theJobSystem = new JobSystem();
	g_theAudio = new AudioSystem();
	g_theConsole = new DevConsole();
	g_theRenderer = new RenderContext();
	g_theEventSystem = new EventSystem();
	g_theDebugRenderSystem = new DebugRender();
	g_theLighthouse = new LighthouseTracking();
	g_theNetwork = new NetworkSystem();
	g_theWindow->SetInputSystem( g_theInput );

	// Start up engine subsystems and game
	g_theRenderer->StartUp( g_theWindow );
	g_theLighthouse->StartUp( g_theWindow );
	g_theGame->StartUp();	
	g_theInput->StartUp();
	g_theConsole->Startup();
	g_theAudio->StartUp();
	g_theNetwork->StartUp();
	g_theEventSystem->StartUp();


	g_theInput->SetCursorMode( MOUSE_MODE_ABSOLUTE );

	// Initialize debug render system
	g_theDebugRenderSystem->DebugRenderSystemStartup();
	//g_theJobSystem->StartUp();
}


void App::Shutdown()
{
	g_theGame->ShutDown();
	g_theConsole->Shutdown();
	g_theLighthouse->ShutDown();
	g_theRenderer->Shutdown();
	g_theNetwork->ShutDown();
	g_theDebugRenderSystem->DebugRenderSystemShutdown();
	Clock::SystemShutdown();
	//g_theJobSystem->ShutDown();
}

void App::RunFrame()
{
	static double timeLastFrameStarted = GetCurrentTimeSeconds(); // runs once only
	double timeThisFrameStarted = GetCurrentTimeSeconds(); // time this frame began
	double deltaSeconds = timeThisFrameStarted - timeLastFrameStarted;
	timeLastFrameStarted = timeThisFrameStarted;
	if( deltaSeconds > 0.1f ) 
	{
		deltaSeconds = 0.1f;
	}
	BeginFrame(); // For all engine systems( Not the game )
	g_theGame->Update( static_cast<float>( deltaSeconds ) );
	g_theLighthouse->RunMainLoop();
	g_theConsole->Update( (float) deltaSeconds );
	g_theRenderer->UpdateFrameTime( (float) deltaSeconds);
	g_theDebugRenderSystem->DebugRenderBeginFrame();
	Render();												 
	EndFrame();
}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return false;
}

void App::RestartGame()
{
	g_theGame = new Game();
	g_theGame->StartUp();
}

void App::Command_Close()
{
	g_theApp->HandleQuitRequested();
}


void App::BeginFrame()
{
	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame(); 
	g_theWindow->BeginFrame();
	g_theAudio->BeginFrame();
	g_theNetwork->BeginFrame();
	g_theConsole->BeginFrame();
	Clock::GetMaster()->BeginFrame();
}

void App::Render() const
{
	g_theGame->Render(); // render game (will clear color)
	g_theConsole->Render( *g_theRenderer, g_theGame->m_uiCamera, 15.f ); // render console, draws over top of game, using its own game
}

void App::EndFrame()
{
	g_theGame->EndFrame();
	g_theRenderer->EndFrame();
	g_theLighthouse->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
	g_theNetwork->EndFrame();
	g_theConsole->EndFrame();
	g_theDebugRenderSystem->DebugRenderEndFrame();
}


COMMAND( QUIT, " " )
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
}



