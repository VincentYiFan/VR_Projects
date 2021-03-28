#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Platform/Window.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define WIN32_LEAN_AND_MEAN		
#include <windows.h>			

const unsigned char KEY_F1			= 112;
const unsigned char KEY_F2			= 113;
const unsigned char KEY_F3			= 114;
const unsigned char KEY_F4			= 115;
const unsigned char KEY_F5			= 116;
const unsigned char KEY_F6			= 117;
const unsigned char KEY_F7			= 118;
const unsigned char KEY_F8			= 119;
const unsigned char KEY_F9			= 120;
const unsigned char KEY_F10			= 121;
const unsigned char KEY_F11			= 122;
const unsigned char KEY_ESC			= 27;
const unsigned char KEY_Q			= 81;
const unsigned char KEY_E			= 69;
const unsigned char KEY_W			= 87;
const unsigned char KEY_A			= 65;
const unsigned char KEY_S			= 83;
const unsigned char KEY_D			= 68;
const unsigned char KEY_C			= 67;
const unsigned char KEY_P			= 80;
const unsigned char KEY_T			= 84;
const unsigned char KEY_Y			= 89;
const unsigned char KEY_N			= 78;
const unsigned char KEY_M			= 77;
const unsigned char KEY_G			= 71;
const unsigned char KEY_H			= 72;
const unsigned char KEY_SPACEBAR	= 32;
const unsigned char KEY_1			= 49;
const unsigned char KEY_2			= 50;
const unsigned char KEY_3			= 51;
const unsigned char KEY_4			= 52;
const unsigned char KEY_5			= 53;
const unsigned char KEY_6			= 54;
const unsigned char KEY_7			= 55;
const unsigned char KEY_8			= 56;
const unsigned char KEY_9			= 57;
const unsigned char KEY_0			= 48;
const unsigned char KEY_PLUS		= 187;
const unsigned char KEY_MINUS		= 189;
const unsigned char KEY_TLIDE		= 192;
const unsigned char KEY_UPARROW		= 38;
const unsigned char KEY_LEFTARROW	= 37;
const unsigned char KEY_DOWNARROW   = 40;
const unsigned char KEY_RIGHTARROW  = 39;
const unsigned char KEY_ENTER		= 13;
const unsigned char KEY_BACKSPACE	= VK_BACK;
const unsigned char KEY_DELETE		= VK_DELETE;
const unsigned char KEY_LEFT_SHIFT  = 16;
const unsigned char KEY_HOME		= 36;
const unsigned char KEY_END			= 35;
const unsigned char KEY_CTRL		= 17;
const unsigned char KEY_PASTE		= 0x16;
const unsigned char KEY_COPY		= 0x03;
const unsigned char KEY_CUT			= 0x18;
const unsigned char KEY_CTRL_A		= 0x01;
const unsigned char KEY_LESS_THAN	= 188;
const unsigned char KEY_GREATER		= 190;
const unsigned char KEY_LBRACKET	= 219;
const unsigned char KEY_RBRACKET	= 221;
const unsigned char KEY_COLON		= 186;
const unsigned char KEY_APOSTROPHE	= 222;

extern Window* g_theWindow;


InputSystem::InputSystem()
{

}
InputSystem::~InputSystem()
{
}

void InputSystem::StartUp()
{

}

void InputSystem::BeginFrame()
{
	m_controllers[0].Update(); 

	if( m_mouseMode == MOUSE_MODE_ABSOLUTE ) {
		UpdateMouse();	
	}
	else {
		UpdateRelativeMode();
	}

}

void InputSystem::EndFrame()
{
	for( int i = 0; i < NUM_KEYCODES; i++ )
	{
		m_keyStates[i].UpdateStatus( m_keyStates[i].IsPressed() );
	}

	for( int index = 0; index < NUM_MOUSE_BUTTONS; index++ )
	{
		m_mouseStates[index].UpdateStatus( m_mouseStates[index].IsPressed() );
	}

	m_scrollAmount = 0.f;
}

void InputSystem::ShutDown()
{

}

bool InputSystem::HandleKeyPressed( unsigned char keyCode )
{
	m_keyStates[keyCode].UpdateStatus( true );
	return true;
}

bool InputSystem::HandleKeyReleased( unsigned char keyCode )
{
	m_keyStates[keyCode].UpdateStatus( false );
	return true;
}


bool InputSystem::IsKeyPressed( unsigned char keyCode )
{
	return m_keyStates[keyCode].IsPressed();
}

bool InputSystem::WasKeyJustPressed( unsigned char keyCode )
{
	return m_keyStates[keyCode].WasJustPressed();
}

bool InputSystem::WasKeyJustReleased( unsigned char keyCode )
{
	return m_keyStates[keyCode].WasJustReleased();
}

bool InputSystem::HandleMouseButtonPressed( eMouseButton button )
{
	m_mouseStates[button].UpdateStatus( true );
	return true;
}

bool InputSystem::HandleMouseButtonReleased( eMouseButton button )
{
	m_mouseStates[button].UpdateStatus( false );
	return true;
}

bool InputSystem::IsMouseButtonPressed( eMouseButton button )
{
	return m_mouseStates[ button ].IsPressed();
}

bool InputSystem::WasMouseButtonJustPressed( eMouseButton button )
{
	return m_mouseStates[ button ].WasJustPressed();
}

bool InputSystem::WasMouseButtonJustReleased( eMouseButton button )
{
	return m_mouseStates[ button ].WasJustReleased();
}

void InputSystem::UpdateMouseButtonState( bool leftButtonDown, bool middleButtonDown, bool rightButtonDown )
{
	m_mouseStates[ LEFT_BUTTON ].UpdateStatus( leftButtonDown );
	m_mouseStates[ MIDDLE_BUTTON ].UpdateStatus( middleButtonDown );
	m_mouseStates[ RIGHT_BUTTON ].UpdateStatus( rightButtonDown );
}

void InputSystem::AddMouseWheelScrollAmount( float scrollAmount )
{
	m_scrollAmount = scrollAmount;
}


/*const */XboxController& InputSystem::GetXboxController( int controllerID )
{
	return m_controllers[controllerID];
}

const KeyButtonState& InputSystem::GetKeyButtonState( unsigned char KeyCode )
{
	return m_keyStates[KeyCode];
}

void InputSystem::PushCharacter( char c )
{
	if( c >= 32 && c <= 122 ) 
	{
		m_characters.push(c);
	}

	if( c == 8 ) { // BackSpace
		g_theEventSystem->FireEvent( "BackSpace" );
	}

	//if( c == 3 ) { // Copy
	//	g_theEventSystem->FireEvent( "COPY" );
	//}

	if( c == KEY_PASTE ) { 
		g_theEventSystem->FireEvent( "PASTE" );
	}
}

bool InputSystem::PopCharacter( char* out )
{
	if( !m_characters.empty() )
	{
		*out = m_characters.front();
		m_characters.pop();
		return true;
	} 
	else {
		return false;
	}
}

void InputSystem::UpdateMouse()
{
	HWND hwnd = (HWND)g_theWindow->m_hwnd;

	POINT mousePos;
	GetCursorPos( &mousePos ); 
	ScreenToClient( hwnd, &mousePos );
	Vec2 mouseClientPos( (float) mousePos.x, (float) mousePos.y );

	RECT clientRect;
	GetClientRect( hwnd, &clientRect );
	AABB2 clientBounds( (float) clientRect.left, (float)clientRect.top, (float)clientRect.right, (float)clientRect.bottom );
	m_mouseNormalizedClientPos = clientBounds.GetUVForPoint( mouseClientPos );
	m_mouseNormalizedClientPos.y = 1.f  -  m_mouseNormalizedClientPos.y; // Flip the mouse vertically
}

void InputSystem::UpdateRelativeMode()
{
	HWND hwnd = (HWND)g_theWindow->m_hwnd;

	POINT offset = { 0, 0 };
	ScreenToClient( hwnd, &offset );
	Vec2 clientToScreenOffset = Vec2( (float)offset.x, (float)offset.y );

	POINT positionThisFrame = { 0, 0 };
	GetCursorPos( &positionThisFrame );
	ScreenToClient( hwnd, &positionThisFrame );
	m_relativeMovement = Vec2((float)positionThisFrame.x, (float)positionThisFrame.y ) - m_positionLastFrame; 

	
	RECT clientRect;
	GetClientRect( hwnd, &clientRect );

	AABB2 clientBounds( (float) clientRect.left , (float)clientRect.top, (float)clientRect.right, (float)clientRect.bottom );

	Vec2 windowCenter = clientBounds.GetCenter() - clientToScreenOffset;

	SetCursorPos( (int)windowCenter.x, (int)windowCenter.y );

	POINT point;
	GetCursorPos( &point );
	windowCenter = Vec2( (float)point.x, (float)point.y );

	m_positionLastFrame = windowCenter + clientToScreenOffset; 
}

void InputSystem::SetCursorMode( eMousePositionMode mode ) 
{
	m_mouseMode = mode;

	// Hide the cursor
	if( mode == MOUSE_MODE_RELATIVE )
	{
		while( ShowCursor( false ) > 0 ){}
		ClipUserCursor( true );
	}
	else 
	{
		while( ShowCursor( true ) < 0 ){}
		ClipUserCursor( false );
	}
}

void InputSystem::ClipUserCursor( bool isClipping )
{
	if( !g_theWindow )
	{
		return;
	}

	HWND hwnd = (HWND)g_theWindow->m_hwnd;

	//RECT rcOldClip;        // previous area for ClipCursor
	RECT newClientRect;       // new area for ClipCursor

	// Record the area in which the cursor can move. 
	//GetClipCursor( &rcOldClip );

	
	// Get the dimensions of the application's window. 
	GetWindowRect( hwnd, &newClientRect );

	// Confine the cursor to the application's window. 
	
	if( isClipping == true )
	{
		ClipCursor( &newClientRect );
	}
	else
	{
		ClipCursor( NULL );
	}
}

void InputSystem::ClearCharacters()
{
	std::queue<char> empty;
	std::swap( m_characters, empty );
}




 