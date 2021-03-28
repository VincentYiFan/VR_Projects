#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Platform//Window.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

App* g_theApp = nullptr;
Window* g_theWindow = nullptr;


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move each of these items to its proper place, once that place is established
// 
//HWND g_hWnd = nullptr;								// ...becomes WindowContext::m_windowHandle
//HDC g_displayDeviceContext = nullptr;				// ...becomes WindowContext::m_displayContext
//HGLRC g_openGLRenderingContext = nullptr;			// ...becomes RenderContext::m_apiRenderingContext
const char* APP_NAME = "DoomensteinVR";	// ...becomes ??? (Change this per project!)



//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	g_gameConfigBlackboard.PopulateFromXmlFile( "Data/GameConfig.xml" );

	const float theWindowAspect = g_gameConfigBlackboard.GetValue( "windowAspect", 2.0f );
	const std::string theWindowAspectString = std::to_string( theWindowAspect );
	const std::string theStartLevel = g_gameConfigBlackboard.GetValue( "startLevel", "LevelOne" );
	const std::string isFullScreen = g_gameConfigBlackboard.GetValue( "isFullScreen", "false" ); 
	const std::string coordinate = g_gameConfigBlackboard.GetValue( "coordinate", "XnZY" ); 
	

	UNUSED( commandLineString );
	UNUSED( applicationInstanceHandle );


	g_theWindow = new Window();
	g_theWindow->Open( APP_NAME, theWindowAspect, 0.85f );


	g_theApp = new App();
	g_theApp->Startup();
	

	// Program main loop; keep running frames until it's time to quit
	while( !g_theApp->IsQuitting() )			
	{		
		g_theWindow->BeginFrame();
		g_theApp->RunFrame();					
	}

	g_theApp->Shutdown();				
	delete g_theApp;
	g_theApp = nullptr;
	g_theWindow = nullptr;

	return 0;
}

