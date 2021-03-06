#pragma once
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/MouseButtonState.hpp"
#include <queue>

constexpr int MAX_XBOX_CONTROLLERS = 4;
constexpr int NUM_KEYCODES  = 250; 
constexpr int NUM_MOUSE_BUTTONS  = 3; 


// Key codes (platform-agnostic)
extern const unsigned char KEY_LEFT_SHIFT;
extern const unsigned char KEY_ESC;
extern const unsigned char KEY_ENTER;
extern const unsigned char KEY_SPACEBAR;
extern const unsigned char KEY_DELETE;
extern const unsigned char KEY_BACKSPACE;
extern const unsigned char KEY_DELETE;
extern const unsigned char KEY_UPARROW;
extern const unsigned char KEY_LEFTARROW;
extern const unsigned char KEY_DOWNARROW;
extern const unsigned char KEY_RIGHTARROW;
extern const unsigned char KEY_F1;
extern const unsigned char KEY_F2;
extern const unsigned char KEY_F3;
extern const unsigned char KEY_F4;
extern const unsigned char KEY_F5;
extern const unsigned char KEY_F6;
extern const unsigned char KEY_F7;
extern const unsigned char KEY_F8;
extern const unsigned char KEY_F9;
extern const unsigned char KEY_F10;
extern const unsigned char KEY_F11;
extern const unsigned char KEY_F12;
extern const unsigned char KEY_Q;
extern const unsigned char KEY_W;
extern const unsigned char KEY_A;
extern const unsigned char KEY_E;
extern const unsigned char KEY_S;
extern const unsigned char KEY_D;
extern const unsigned char KEY_C;
extern const unsigned char KEY_P;
extern const unsigned char KEY_T;
extern const unsigned char KEY_Y;
extern const unsigned char KEY_N;
extern const unsigned char KEY_M;
extern const unsigned char KEY_G;
extern const unsigned char KEY_H;
extern const unsigned char KEY_1;
extern const unsigned char KEY_2;
extern const unsigned char KEY_3;
extern const unsigned char KEY_4;
extern const unsigned char KEY_5;
extern const unsigned char KEY_6;
extern const unsigned char KEY_7;
extern const unsigned char KEY_8;
extern const unsigned char KEY_9;
extern const unsigned char KEY_0;
extern const unsigned char KEY_PLUS;
extern const unsigned char KEY_MINUS;
extern const unsigned char KEY_TLIDE;
extern const unsigned char KEY_HOME;
extern const unsigned char KEY_END;
extern const unsigned char KEY_CTRL;
extern const unsigned char KEY_PASTE;
extern const unsigned char KEY_COPY;
extern const unsigned char KEY_CUT;
extern const unsigned char KEY_CTRL_A;
extern const unsigned char KEY_LBRACKET;
extern const unsigned char KEY_RBRACKET;
extern const unsigned char KEY_LESS_THAN;
extern const unsigned char KEY_GREATER;
extern const unsigned char KEY_COLON;
extern const unsigned char KEY_APOSTROPHE;

enum eMouseButton: unsigned char
{
	LEFT_BUTTON,
	MIDDLE_BUTTON,
	RIGHT_BUTTON
};
enum eMousePositionMode : unsigned char 
{
	MOUSE_MODE_RELATIVE,
	MOUSE_MODE_ABSOLUTE,
};


//-------------------------------------------------------------------------------
class InputSystem 
{
public:
	InputSystem();
	~InputSystem();

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	bool HandleKeyPressed ( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );

	bool IsKeyPressed ( unsigned char keyCode );
	bool WasKeyJustPressed( unsigned char keyCode );
	bool WasKeyJustReleased( unsigned char keyCode );

	//-------------------------------------------------------------------------------------------------------
	bool HandleMouseButtonPressed( eMouseButton button );
	bool HandleMouseButtonReleased( eMouseButton button );

	bool IsMouseButtonPressed( eMouseButton button );
	bool WasMouseButtonJustPressed( eMouseButton button );
	bool WasMouseButtonJustReleased( eMouseButton button );

	void UpdateMouseButtonState( bool leftButtonDown, bool middleButtonDown, bool rightButtonDown ); 
	void AddMouseWheelScrollAmount( float scrollAmount );


	Vec2 GetMouseNormalizedClientPos() const { return m_mouseNormalizedClientPos; }

	XboxController& GetXboxController( int controllerID );
	const KeyButtonState& GetKeyButtonState( unsigned char KeyCode );

public:
	// For typing
	void PushCharacter( char c );
	bool PopCharacter( char* out );

public:
	void UpdateMouse();
	void UpdateRelativeMode();
	//void UpdateMouseState( bool leftButtonState, bool middleButtonState, bool rightButtonState );

	void SetCursorMode( eMousePositionMode mode );
	void ClipUserCursor( bool isClipping );

	void ClearCharacters();

private:
	KeyButtonState m_keyStates[ NUM_KEYCODES ];

	MouseButtonState m_mouseStates[ NUM_MOUSE_BUTTONS ];

	XboxController m_controllers[MAX_XBOX_CONTROLLERS] =
	{
		XboxController(0),
		XboxController(1),
		XboxController(2),
		XboxController(3)
	};

public:
	Vec2 m_mouseNormalizedClientPos = Vec2::ZERO;

	float m_scrollAmount = 0.f;
	Vec2 m_relativeMovement = Vec2::ZERO;
	Vec2 m_positionLastFrame = Vec2::ZERO;

	eMousePositionMode m_mouseMode = MOUSE_MODE_ABSOLUTE;

public:
	std::queue<char> m_characters; 
};
