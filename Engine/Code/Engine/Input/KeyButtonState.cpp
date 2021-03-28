#include "KeyButtonState.hpp"

void KeyButtonState::UpdateStatus( bool isNowPressed )
{
	m_wasPressedLastFrame = m_isPressed;
	m_isPressed = isNowPressed;
}

bool KeyButtonState::WasJustPressed() const
{
	return m_isPressed && !m_wasPressedLastFrame;	
}


bool KeyButtonState::WasJustReleased() const
{
	return !m_isPressed && m_wasPressedLastFrame;	
}

void KeyButtonState::Reset()
{
	m_isPressed = false;
	m_wasPressedLastFrame = false;
}
