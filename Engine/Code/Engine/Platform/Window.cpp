#include "Engine/Platform/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/resource.h"

static TCHAR const* WND_CLASS_NAME = L"Simple Window Class";

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//
// #SD1ToDo: We will move this function to a more appropriate place later on...
//
static LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	Window* window = (Window*) ::GetWindowLongPtr( windowHandle, GWLP_USERDATA );

	switch( wmMessageCode )
	{
		case WM_ACTIVATE:
		{
			InputSystem* input = window->GetInputSystem();
			switch( wParam ) 
			{
			case WA_INACTIVE: {
				if( input )
				{
					input->SetCursorMode( MOUSE_MODE_ABSOLUTE );
				}
			} break;

			case WA_ACTIVE:
			case WA_CLICKACTIVE:
			{
				if( input ) 
				{
					// Read GameConfig to determine which Cursor Mode
					std::string cursorModeName = g_gameConfigBlackboard.GetValue( "cursorMode", "Relative" );
					//if( cursorModeName == "Relative" && !g_theConsole->IsOpen() ) 
					//{
					//	input->SetCursorMode( MOUSE_MODE_RELATIVE );
					//}
					//else 
					//{
					//	input->SetCursorMode( MOUSE_MODE_ABSOLUTE );
					//}
				}
			} break;
			}

			return 0;
		}

		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			if( MessageBox( windowHandle, L"Really quit?", L"My application", MB_OKCANCEL ) == IDOK )
			{
				g_theEventSystem->FireEvent( "QUIT" );
				DestroyWindow( windowHandle );
			}
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
		}

		case WM_DESTROY:
		{
			PostQuitMessage( 0 );
			return 0;
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char)wParam;
			InputSystem* input = window->GetInputSystem();
			input->HandleKeyPressed(asKey);
			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char)wParam;
			InputSystem* input = window->GetInputSystem();
			input->HandleKeyReleased( asKey );
			break;
		}

		case WM_CHAR:
		{
			wchar_t character = (wchar_t)wParam;
			// g_theDevConsole->OnKeyPress( character ); 
			g_theInput->PushCharacter( (char)character );
			break;
		}

		case WM_SYSKEYDOWN:
		{
			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			bool leftButtonDown = wParam & MK_LBUTTON;
			bool middleButtonDown = wParam & MK_MBUTTON;
			bool rightButtonDown = wParam & MK_RBUTTON;

			g_theInput->UpdateMouseButtonState( leftButtonDown, middleButtonDown, rightButtonDown );

			//g_theInput->HandleMouseButtonPressed( LEFT_BUTTON );
			break;
		}

		case WM_MOUSEWHEEL:
		{
			short scrollFixedPoint = (short)(wParam >> 16); // shift away low word part, leaving only the highword
			float scrollAmount = (float)scrollFixedPoint / 120.0f; // convert to a numeric value
			g_theInput->AddMouseWheelScrollAmount( scrollAmount ); // let the input system know that the mouse wheel has moved - again, interface design is up to you - this is just a suggestion
		}

	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}

static void RegisterWindowClass()
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	//windowClassDescription.hIcon = NULL;
	windowClassDescription.hIcon = static_cast<HICON>( LoadImage( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 32, 32, 0  ) );
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = WND_CLASS_NAME;
	::RegisterClassEx( &windowClassDescription );
}

static void UnregisterWindowClass()
{
	::UnregisterClass( WND_CLASS_NAME, GetModuleHandle( NULL ) );
}

Window::Window()
{
	RegisterWindowClass();
}

Window::~Window()
{
	UnregisterWindowClass();
}

bool Window::Open( std::string const& title, float clientAspect, float maxClientFractionOfDesktop /*00.90f*/ )
{
	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	if( clientAspect > desktopAspect )
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * clientAspect;
	}

	m_clientWidth = (UINT) clientWidth;
	m_clientHeight = (UINT) clientHeight;

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[1024];
	MultiByteToWideChar( GetACP(), 0, title.c_str(), -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[0] ) );
	HWND hwnd = CreateWindowEx(
		windowStyleExFlags,
		WND_CLASS_NAME,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		(HINSTANCE)::GetModuleHandle( NULL ),
		NULL );

	::SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)this ); // associate window using this window

	if( hwnd == nullptr )
	{
		return false;
	}

	ShowWindow( hwnd, SW_SHOW );
	SetForegroundWindow( hwnd );
	SetFocus( hwnd );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );

	m_hwnd = (void*)hwnd;

	return true;
}

void Window::Close()
{
	HWND hwnd = (HWND) m_hwnd;
	if( NULL == hwnd )
	{
		return;
	}

	::DestroyWindow( hwnd );
	m_hwnd = nullptr;
}

void Window::SetInputSystem( InputSystem* input )
{
	m_input = input;
}

void Window::BeginFrame()
{
	MSG queuedMessage;
	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage ); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}
