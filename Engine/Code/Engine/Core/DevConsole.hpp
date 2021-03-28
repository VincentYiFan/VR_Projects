 #pragma once
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include <vector>
#include <map>
 
class RenderContext;
class Camera;
class BitmapFont;

extern BitmapFont*  g_theFont;

struct ColoredLine
{
	Rgba8 m_color;
	std::string m_text;
};

class DevConsole
{
public:
	DevConsole();
	void Startup();
	void BeginFrame();
	void Update( float deltaSeconds );
	void UpdateKeyFunctions();
	void EndFrame();
	void Shutdown();

	void ProcessInput();
	void AddCharacterToInput( char c );
	void ExecuteCommand( std::string command );
	void AutoSelect();
	void AddToCommandHistory( std::string commandName );
	void PrintString( const Rgba8& textColor, const std::string& devConsolePrintString );
	void Render( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font = g_theFont ) ;
	void RenderCaret( RenderContext& renderer, float lineHeight, float caretWidth = 2.f );
	void SetIsOpen( bool isOpen );
	bool IsOpen() const;

	// Error Function
	void Error( const char* messageFormat, ... );
	void Printf( const char* messageFormat, ... );

public:
	std::string GetClipboardText();


public:
	std::string m_textBox;
	std::string m_autoSelectTextBox;
	bool m_isShowingAutoSelectText = false;
	bool m_hasFinishAutoComplete = false;

	bool m_isPressingAltKey = false;

	std::map< std::string, std::string > m_typeCommands;
	std::vector<std::string> m_commandNames;

	std::vector<std::string> m_commandHistory;
	int m_cmHistoryIdx = 0;

	std::vector<Rgba8> m_textColors;
	std::vector<std::string> m_devConsolePrintStrings;
	bool m_isActive = false;

	// Caret
	Rgba8 m_caretColor = Rgba8::WHITE;
	int m_caretPosIndex = 0;
	float m_opacityReference = 0.f;
};

void Command_BackSpace();
