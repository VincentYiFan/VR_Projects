#include <math.h>
#include "XboxController.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> 
#include <Xinput.h>
#include "Engine/Math/MathUtils.hpp"

#pragma comment( lib, "xinput9_1_0" )

KeyButtonState* m_keyButton = nullptr;

XboxController::XboxController( int controllerID )
	:m_controllerID(controllerID)
{

}

XboxController::~XboxController()
{
}

const KeyButtonState& XboxController::GetButtonState( XboxButtonID buttonID ) const
{
	return m_buttonStates[buttonID];
}

void XboxController::Update()
{
	// Read raw controller state directly from XInput API
	XINPUT_STATE xboxControllerState;    // Current state
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ));  // The function works similar as ZeroMemory, clear state every update()
	DWORD errorStatus = XInputGetState( m_controllerID, &xboxControllerState );  // Place all the button pressed into state

	if( errorStatus == ERROR_SUCCESS ) 
	{
		
		m_leftVibrationFraction -= 0.01f;
		m_rightVibrationFraction -= 0.01f;
		m_leftVibrationFraction = Clamp(m_leftVibrationFraction, 0.f, 1.f);
		m_rightVibrationFraction = Clamp(m_rightVibrationFraction, 0.f, 1.f);
		Rumble(m_leftVibrationFraction, m_rightVibrationFraction);
		
		m_isConnected = true;
		XINPUT_GAMEPAD& gamepad = xboxControllerState.Gamepad;
		UpdateJoystick( m_leftJoystick, gamepad.sThumbLX, gamepad.sThumbLY );
		UpdateJoystick( m_rightJoystick, gamepad.sThumbRX, gamepad.sThumbRY );
		UpdateTrigger( m_leftTriggerValue, gamepad.bLeftTrigger);
		UpdateTrigger( m_rightTriggerValue, gamepad.bRightTrigger );
		UpdateButton( XBOX_BUTTON_ID_A,               gamepad.wButtons, XINPUT_GAMEPAD_A );
		UpdateButton( XBOX_BUTTON_ID_B,				  gamepad.wButtons, XINPUT_GAMEPAD_B );
		UpdateButton( XBOX_BUTTON_ID_X,				  gamepad.wButtons, XINPUT_GAMEPAD_X );
		UpdateButton( XBOX_BUTTON_ID_Y,				  gamepad.wButtons, XINPUT_GAMEPAD_Y );
		UpdateButton( XBOX_BUTTON_ID_BACK,			  gamepad.wButtons, XINPUT_GAMEPAD_BACK );
		UpdateButton( XBOX_BUTTON_ID_START,			  gamepad.wButtons, XINPUT_GAMEPAD_START );
		UpdateButton( XBOX_BUTTON_ID_LSHOULDER,		  gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER );
		UpdateButton( XBOX_BUTTON_ID_RSHOULDER,		  gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER );
		UpdateButton( XBOX_BUTTON_ID_LTHUMB,		  gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB );
		UpdateButton( XBOX_BUTTON_ID_RTHUMB,		  gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB );
		UpdateButton( XBOX_BUTTON_ID_DPAD_RIGHT,	  gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT );
		UpdateButton( XBOX_BUTTON_ID_DPAD_UP,		  gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP );
		UpdateButton( XBOX_BUTTON_ID_DPAD_LEFT,		  gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT );
		UpdateButton( XBOX_BUTTON_ID_DPAD_DOWN,		  gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN );
	}

	else if( errorStatus ==ERROR_DEVICE_NOT_CONNECTED )
	{
		m_isConnected = false;
		Reset();
	}
}

void XboxController::Reset()
{
	m_leftTriggerValue = 0.f;
	m_rightTriggerValue = 0.f;	
}

void XboxController::UpdateTrigger( float& triggerValue, unsigned char rawValue )
{
	triggerValue = static_cast<float>(rawValue) / 255.f;
}

void XboxController::UpdateJoystick( AnalogJoystick& joystick, short rawX, short rawY )
{
	
	joystick.m_rawPosition = Vec2( rawX, rawY );
	float normLX = fmaxf( -1, (float)joystick.m_rawPosition.x / 32767 );
	float normLY = fmaxf( -1, (float)joystick.m_rawPosition.y / 32767 );
	joystick.UpdatePosition( normLX, normLY );
}

void XboxController::UpdateButton( XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag )
{
	if( (buttonFlags & buttonFlag) != 0 ) 
	{
		m_buttonStates[buttonID].UpdateStatus(true);	
	}
	else {
		m_buttonStates[buttonID].UpdateStatus(false);	
	}
	return;
}

void XboxController::Rumble( float a_fLeftMotor, float a_fRightMotor )
{
	// Vibration state
	XINPUT_VIBRATION VibrationState;

	// Zero memory
	ZeroMemory( &VibrationState, sizeof( XINPUT_VIBRATION ) );

	// Calculate vibration values
	int iLeftMotor  = int( a_fLeftMotor * 65535.0f );
	int iRightMotor = int( a_fRightMotor * 65535.0f );

	
	// Set vibration values
	VibrationState.wLeftMotorSpeed  = (WORD)iLeftMotor;
	VibrationState.wRightMotorSpeed = (WORD)iRightMotor;

	// Set the vibration state
	XInputSetState( m_controllerID, &VibrationState );
}

void XboxController::AddVibrationFraction( float fraction )
{
	float clampedFraction = Clamp( fraction, 0.f, 1.f );
	m_leftVibrationFraction += clampedFraction;
	m_rightVibrationFraction += clampedFraction;
}


