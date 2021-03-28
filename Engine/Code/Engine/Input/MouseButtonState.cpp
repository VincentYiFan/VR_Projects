#include "MouseButtonState.hpp"

void MouseButtonState::UpdateStatus( bool isNowPressed )
{
	m_wasPressedLastFrame = m_isPressed;
	m_isPressed = isNowPressed;
}

bool MouseButtonState::WasJustPressed() const
{
	return m_isPressed && !m_wasPressedLastFrame;	
}

bool MouseButtonState::WasJustReleased() const
{
	return !m_isPressed && m_wasPressedLastFrame;	
}

void MouseButtonState::Reset()
{
	m_isPressed = false;
	m_wasPressedLastFrame = false;
}
