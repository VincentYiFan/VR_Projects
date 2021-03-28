#pragma once
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <string>

#define WM_CUT		0x0300
#define WM_COPY		0x0301
#define WM_PASTE    0x0302
#define WM_CLEAR	0x0303

typedef unsigned int uint;


//-------------------------------------------------------------------------------------------------------------------
class InputSystem;
class EventSystem;
//-------------------------------------------------------------------------------------------------------------------

class Window
{
public:
	Window();
	~Window();

	bool Open( std::string const& title, float aspectRatio, float maxClientFractionOfDesktop /*0.90f*/ );
	void Close();

	void SetInputSystem( InputSystem* input );
	InputSystem* GetInputSystem() const { return m_input; }

	void BeginFrame();

	uint GetClientWidth() { return m_clientWidth; }
	uint GetClientHeight() { return m_clientHeight; }


public:
	void* m_hwnd = nullptr;

	uint m_clientWidth = 0;
	uint m_clientHeight = 0;

	InputSystem* m_input = nullptr;
};
