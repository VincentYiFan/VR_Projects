#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include <stdio.h>      /* printf */
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

extern DevConsole* g_theConsole;

DevConsole::DevConsole()
{
}

void DevConsole::Startup()
{
	m_typeCommands["help"] = "Show all command functions";
	m_typeCommands["quit"] = "Close the application";

	// temp
	m_typeCommands["warp"] = "Show all map commands";

	//m_commandNames.push_back( "help" );
	//m_commandNames.push_back( "quit" );
}

void DevConsole::BeginFrame()
{
}

void DevConsole::Update( float deltaSeconds )
{
	if( !m_isActive ) {
		return;
	}
	ProcessInput();
	AutoSelect();
	UpdateKeyFunctions();

	m_opacityReference += deltaSeconds;
	unsigned char a = (unsigned char)( sin( (double)m_opacityReference * 5 ) * 128 + 127 );
	m_caretColor = Rgba8( 255, 255, 255, a );
 }

void DevConsole::UpdateKeyFunctions()
{
	if( !m_isActive ) {
		return;
	}

	if( g_theInput->WasKeyJustPressed( KEY_ENTER ) && (m_textBox.size() > 0) ) 
	{
		//if( m_isShowingAutoSelectText == false ) {
		//	m_hasFinishAutoComplete = true;
		//}
		//
		//
		//if( m_hasFinishAutoComplete == false ) 
		//{
		//	m_textBox = m_autoSelectTextBox;
		//	m_autoSelectTextBox.clear();
		//	m_isShowingAutoSelectText = false;
		//	m_caretPosIndex = (int)m_textBox.length();
		//	m_hasFinishAutoComplete = true;
		//}		
		//
		//else
		//{
		//	m_isShowingAutoSelectText = false;
			PrintString( Rgba8::YELLOW, m_textBox );
			ExecuteCommand( m_textBox );
			AddToCommandHistory( m_textBox );
			m_textBox.clear();
			m_caretPosIndex = 0;
		//	m_hasFinishAutoComplete = false;
		//}
	}

	if( g_theInput->WasKeyJustPressed( KEY_DELETE ) && m_caretPosIndex < (int)m_textBox.size() ) {
		m_textBox.erase( m_caretPosIndex, 1 );
	}


	if( g_theInput->WasKeyJustPressed( KEY_LEFTARROW ) && m_caretPosIndex > 0 ) {
		m_caretPosIndex--;
	}

	if( g_theInput->WasKeyJustPressed( KEY_RIGHTARROW ) && m_caretPosIndex < (int)m_textBox.size() ) {
		m_caretPosIndex++;
	}

	if( g_theInput->WasKeyJustPressed( KEY_UPARROW ) && m_commandHistory.size() > 0 && (m_cmHistoryIdx > 0) ) {
		--m_cmHistoryIdx;
		m_textBox = m_commandHistory[ m_cmHistoryIdx ];
		m_caretPosIndex = (int)m_textBox.size();
	}

	if( g_theInput->WasKeyJustPressed( KEY_DOWNARROW ) && m_commandHistory.size() > 0 && (m_cmHistoryIdx < m_commandHistory.size() - 1 ) ) {
		++m_cmHistoryIdx;
		m_textBox = m_commandHistory[m_cmHistoryIdx];
		m_caretPosIndex = (int)m_textBox.size();
	}

	if( g_theInput->WasKeyJustPressed( KEY_HOME ) ) {
		m_caretPosIndex = 0;
	}

	if( g_theInput->WasKeyJustPressed( KEY_END ) && m_textBox.size() > 0 ) {
		m_caretPosIndex = (int)m_textBox.size();
	}

	if( g_theInput->IsKeyPressed( KEY_CTRL ) ) {
		if( g_theInput->WasKeyJustPressed( KEY_LEFTARROW ) )
		{
			while( (m_caretPosIndex > 0) && (m_caretPosIndex < (int)m_textBox.size())
				&& !(m_textBox[m_caretPosIndex - 1] == 32 && m_textBox[m_caretPosIndex] != 32) )
			{
				m_caretPosIndex--;
			}
		}
		if( g_theInput->WasKeyJustPressed( KEY_RIGHTARROW ) )
		{
			while( (m_caretPosIndex > 0) && (m_caretPosIndex < (int)m_textBox.size())
				&& !(m_textBox[m_caretPosIndex - 1]  != 32 && m_textBox[m_caretPosIndex]  == 32) )
			{
				m_caretPosIndex++;
			}
		}
	}
}

void DevConsole::EndFrame()
{
}

void DevConsole::Shutdown()
{
}

void DevConsole::ProcessInput()
{
	char c;
	while( g_theInput->PopCharacter( &c ) && m_isActive ) {
		AddCharacterToInput( c );
	}
}

void DevConsole::AddCharacterToInput( char c )
{
	m_textBox.insert( m_caretPosIndex, 1, c );
	m_caretPosIndex++; // Adjust the position of the caret
}

void DevConsole::ExecuteCommand( std::string command )
{
	Strings strings = SplitStringOnDelimiter( command, ' ' );

	if( strings.size() >= 1 )
	{
		g_theEventSystem->FireEventWithValue( command );
	}
	else 
	{
		if( m_typeCommands.find( command ) != m_typeCommands.end() )
		{
			if( command == "quit" ) 
			{ 
				g_theEventSystem->FireEvent( "QUIT" ); 
			}
			else if( command == "help" ) 
			{
				std::map<std::string, std::string>::iterator it = m_typeCommands.begin();
				while( it != m_typeCommands.end() ) 
				{
					std::string stringToPrint =  " -" + it->first + ": " + m_typeCommands[it->first];
					PrintString( Rgba8::YELLOW, stringToPrint );
					it++;
				}
			}
			else if( command == "warp" )
			{
				g_theConsole->PrintString( Rgba8::WHITE, "Map Commands:" );
				g_theConsole->PrintString( Rgba8::YELLOW, "warp pos=5.5,7.5" );
				g_theConsole->PrintString( Rgba8::YELLOW, "warp map=TwistyMaze" );
				g_theConsole->PrintString( Rgba8::YELLOW, "warp map=EmptyRoom pos=3,7 yaw=90" );
			}
		}
		else 
		{
			std::string str = "ERROR: Unknown Command:" + command + "." + " Type help for a list of commands.";
			PrintString( Rgba8::RED, str );
		}
	}
}

void DevConsole::AutoSelect()
{
	if( m_textBox.size() <= 0 ) 
	{
		if( m_autoSelectTextBox.size() > 0 ) 
		{
			m_autoSelectTextBox.clear();
		}
		return;
	}

	if( m_textBox == m_autoSelectTextBox ) // Testing
	{
		m_isShowingAutoSelectText = false;
		return;
	}

	for( int textBoxCharIdx = 0; textBoxCharIdx < (int)m_textBox.size(); textBoxCharIdx++  )
	{
		for( int commandIdx = 0; commandIdx < (int)m_commandNames.size(); commandIdx++ )
		{
			if( ( textBoxCharIdx < m_commandNames[commandIdx].length() && ( m_textBox.at( textBoxCharIdx ) == m_commandNames[commandIdx].at( textBoxCharIdx ) ) ) )
			{
				m_isShowingAutoSelectText = true;
				m_autoSelectTextBox = m_commandNames[commandIdx];

				for( int j = textBoxCharIdx + 1; j < (int)m_textBox.size(); j++ ) {
					if( j < m_commandNames[commandIdx].length() && m_textBox.at( j ) != m_commandNames[commandIdx].at( j ) ) {
						m_isShowingAutoSelectText = false;
					}
				}
			}
		}
	}
}

void DevConsole::AddToCommandHistory( std::string commandName )
{
	m_commandHistory.push_back( commandName );
	m_cmHistoryIdx = (int)m_commandHistory.size();
}

void DevConsole::PrintString( const Rgba8& textColor, const std::string& devConsolePrintString )
{
	m_textColors.push_back(textColor);
	m_devConsolePrintStrings.push_back(devConsolePrintString);
}

void DevConsole::Render( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font )
{
	UNUSED( camera );

	if( m_isActive )
	{
		renderer.BindTexture( nullptr );
		Vec2 dimensions = renderer.m_swapchain->GetBackBuffer()->GetDimensions();

		std::vector<Vertex_PCU> backgroundVerts;
		//AABB2 background = AABB2( camera.GetOrthoBottonLeft(), camera.GetOrthoTopRight() );
		AABB2 background = AABB2( Vec2::ZERO, dimensions );
		renderer.DrawAABB2( background, Rgba8( 10, 10, 10, 150 ) ); 

		//AABB2 inputTextBoxBackground = AABB2( camera.GetOrthoBottonLeft(), Vec2( camera.GetOrthoTopRight().x, camera.GetOrthoBottonLeft().y + lineHeight  ) );
		AABB2 inputTextBoxBackground = AABB2( Vec2::ZERO, Vec2( dimensions.x, lineHeight ) );
		renderer.DrawAABB2( inputTextBoxBackground, Rgba8( 128, 128, 128, 255 ) );

		renderer.BindTexture( nullptr );
		std::vector<Vertex_PCU> textVerts;
		for( int index = (int)m_devConsolePrintStrings.size() - 1; index >= 0; index-- )
		{
			Vec2 textMins = background.mins + Vec2( 0, lineHeight ) * (float)( m_devConsolePrintStrings.size() /*- 1*/ - index );
			if( (textMins.y + lineHeight) <= background.maxs.y ) // Draws only log print lines that fit on screen 
			{
				//font->AddVertsForText2D( textVerts, textMins, 3.f, m_devConsolePrintStrings[index], m_textColors[index] );
				font->AddVertsForText2D( textVerts, textMins, lineHeight, m_devConsolePrintStrings[index],  m_textColors[index] );
			}
		}

		// Text box
		font->AddVertsForText2D( textVerts, background.mins, lineHeight, m_textBox, Rgba8::WHITE );

		// Auto select box
		if( m_isShowingAutoSelectText )
		{
			font->AddVertsForText2D( textVerts, background.mins, lineHeight, m_autoSelectTextBox, Rgba8( 255, 255, 255, 100 ) );
		}

		renderer.BindTexture( font->GetTexture() );
		renderer.DrawVertexArray( textVerts );

		RenderCaret( renderer, lineHeight );
	}
}

void DevConsole::RenderCaret( RenderContext& renderer, float lineHeight, float caretWidth )
{
	float textWidth = lineHeight; // Equal to height only in this case
	//Vec2 caretMins( textWidth * (float)m_textBox.size() , 0.f );
	Vec2 caretMins( textWidth * m_caretPosIndex , 0.f );
	Vec2 caretMaxs( caretMins.x + caretWidth, lineHeight );
	AABB2 caret = AABB2( caretMins, caretMaxs );

	renderer.BindTexture( nullptr );
	renderer.DrawAABB2( caret, m_caretColor );
}

void DevConsole::SetIsOpen( bool isOpen )
{
	if( isOpen == false ) {
		g_theInput->ClearCharacters();
		m_textBox.clear();
		m_caretPosIndex = 0;
	}
	else
	{
		g_theInput->ClearCharacters();
		m_textBox.clear();
	}

	m_isActive = isOpen;
}

bool DevConsole::IsOpen() const
{
	return m_isActive;
}

std::string DevConsole::GetClipboardText()
{
	// Try opening the clipboard
	if( !OpenClipboard( nullptr ) ) {
		PrintString( Rgba8::RED, "Failed to open clipboard" );
	}

	// Get handle of clipboard object for ANSI text
	HANDLE hData = GetClipboardData( CF_TEXT );
	if( hData == nullptr ) {
		PrintString( Rgba8::RED, "Failed to get clipboard data" );
	}

	// Lock the handle to get the actual text pointer
	char* pszText = static_cast<char*>(GlobalLock( hData ));
	if( pszText == nullptr ) {
		PrintString( Rgba8::RED, "Failed to get the actual text pointer" );
	}

	// Save text in a string class instance
	std::string text( pszText );

	// Release the lock
	GlobalUnlock( hData );

	// Release the clipboard
	CloseClipboard();

	return text;
}

void DevConsole::Error( const char* messageFormat, ... )
{
	char buffer[256];
	va_list args;
	va_start( args, messageFormat );

	// Write formatted data from variable argument list to string
	vsprintf_s( buffer, messageFormat, args );

	std::string str(buffer);
	PrintString( CONSOLE_COLOR_ERROR_MAJOR, str );

	va_end( args );

	// Display mode full
	SetIsOpen( true );
}

void DevConsole::Printf( const char* messageFormat, ... )
{
	char buffer[256];
	va_list args;
	va_start( args, messageFormat );

	// Write formatted data from variable argument list to string
	vsprintf_s( buffer, messageFormat, args );

	std::string str( buffer );
	PrintString( Rgba8::WHITE, str );

	va_end( args );
}

void Command_BackSpace()
{
	if( g_theConsole->m_textBox.size() > 0 && g_theConsole->m_isActive )
	{
		g_theConsole->m_textBox.erase( (int)((double)g_theConsole->m_caretPosIndex - 1), 1 );
		g_theConsole->m_caretPosIndex--;
	}
}

COMMAND( BackSpace, " " ) 
{
	UNUSED(args);
	if( g_theConsole->m_textBox.size() > 0 && g_theConsole->m_isActive )
	{		g_theConsole->m_textBox.erase( (int)((double)g_theConsole->m_caretPosIndex - 1), 1 );
		g_theConsole->m_caretPosIndex--;
	}
}


COMMAND( PASTE, " " )
{
	UNUSED(args);
	g_theConsole->m_textBox = g_theConsole->GetClipboardText();
	g_theConsole->m_caretPosIndex = (int)g_theConsole->m_textBox.size();
	//g_theConsole->PrintString( Rgba8::CYAN, g_theConsole->GetClipboardText() );
}


