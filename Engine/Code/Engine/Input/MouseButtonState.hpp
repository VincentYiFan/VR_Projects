#pragma once

class MouseButtonState
{
public:
	void UpdateStatus( bool isNowPressed );
	bool IsPressed() const { return m_isPressed; }
	bool WasJustPressed() const;
	bool WasJustReleased() const;
	void Reset();

private:
	bool m_isPressed = false;
	bool m_wasPressedLastFrame = false;
};