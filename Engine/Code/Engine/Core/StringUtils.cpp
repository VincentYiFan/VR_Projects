#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <stdarg.h>
#include <iostream>
//#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cctype>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


float GetValueFromString( std::string const& str, float const defValue )
{
	float value = defValue;
	value = stof( str );
	return value;
}

std::string ToString( float value )
{
	return std::to_string(value);
}

std::string ToString( uint value )
{
	return std::to_string( value );
}

std::string ToString( int value )
{
	return std::to_string( value );
}

std::string ToString( double value )
{
	return std::to_string( value );
}

std::string ToString( bool value )
{
	return value ? "true" : "false";
}

std::string ToString( std::string value )
{
	return value;
}

std::string ToString( Vec2 value )
{
	std::string str = Stringf( "%f,%f", value.x, value.y );
	return str;
}

std::string ToString( Vec3 value )
{
	std::string str = Stringf( "%f,%f,%f", value.x, value.y, value.z );
	return str;
}

std::string ToString( IntVec2 value )
{
	std::string str = Stringf( "%i,%i", value.x, value.y );
	return str;
}

std::string ToString( Rgba8 value )
{
	std::string str = Stringf( "%hhu,%hhu,%hhu,%hhu", value.r, value.g, value.b, value.a );
	return str;
}

bool ToBool( std::string str )
{
	//std::transform( str.begin(), str.end(), str.begin(), ::tolower );
	//std::istringstream is( str );
	//bool b;
	//is >> std::boolalpha >> b;
	//return b;

	if( str == "true" || str == "True" || str == "TRUE" )
	{
		return true;
	}
	else if( str == "false" || str == "False" || str == "FALSE" )
	{
		return false;
	}
	
	return false;
}

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

Strings SplitStringOnDelimiter( const std::string& originalString, char delimiterToSplitOn )
{
	Strings splitStrings;
	size_t splitStartIndex = 0;

	for( ;; )
	{
		size_t splitEndIndex = originalString.find( delimiterToSplitOn, splitStartIndex );
		size_t splitLength = splitEndIndex - splitStartIndex;

		std::string subString( originalString, splitStartIndex, splitLength );
		splitStrings.push_back( subString );
		if( splitEndIndex == std::string::npos )
			break;

		splitStartIndex = splitEndIndex + 1;
	}
		return splitStrings;
}

std::string tail( const std::string& in )
{
	size_t token_start = in.find_first_not_of( " \t" );
	size_t space_start = in.find_first_of( " \t", token_start );
	size_t tail_start = in.find_first_not_of( " \t", space_start );
	size_t tail_end = in.find_last_not_of( " \t" );
	if( tail_start != std::string::npos && tail_end != std::string::npos )
	{
		return in.substr( tail_start, tail_end - tail_start + 1 );
	}
	else if( tail_start != std::string::npos )
	{
		return in.substr( tail_start );
	}
	return "";
}

std::string firstToken( const std::string& in )
{
	if( !in.empty() )
	{
		size_t token_start = in.find_first_not_of( " \t" );
		size_t token_end = in.find_first_of( " \t", token_start );
		if( token_start != std::string::npos && token_end != std::string::npos )
		{
			return in.substr( token_start, token_end - token_start );
		}
		else if( token_start != std::string::npos )
		{
			return in.substr( token_start );
		}
	}
	return "";
}

void split( const std::string& in, std::vector<std::string>& out, std::string token )
{
	out.clear();

	std::string temp;

	for( int i = 0; i < int( in.size() ); i++ )
	{
		std::string test = in.substr( i, token.size() );

		if( test == token )
		{
			if( !temp.empty() )
			{
				out.push_back( temp );
				temp.clear();
				i += (int)token.size() - 1;
			}
			else
			{
				out.push_back( "" );
			}
		}
		else if( i + token.size() >= in.size() )
		{
			temp += in.substr( i, token.size() );
			out.push_back( temp );
			break;
		}
		else
		{
			temp += in[i];
		}
	}
}

char const* Parse( float* out, char const* str )
{
	try {
		std::string fs( str );
		*out = std::stof( fs );
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}

char const* Parse( uint* out, char const* str )
{
	try {
		std::string s( str );
		*out = (uint)std::stoul( s );
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}

char const* Parse( int* out, char const* str )
{
	try {
		std::string s( str );
		*out = std::stoi( s );
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}

char const* Parse( double* out, char const* str )
{
	try {
		std::string s( str );
		*out = std::stod( s );
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}

char const* Parse( bool* out, char const* str )
{
	try {
		std::string s( str );
		*out = ToBool( str );
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}

char const* Parse( std::string* out, char const* str )
{
	try {
		std::string s( str );
		*out = s;
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}

char const* Parse( Vec2* out, char const* str )
{
	try {
		std::string s( str );
		Strings strings = SplitStringOnDelimiter( s, ',' );
		*out = Vec2( stof(strings[0]), stof(strings[1]) );
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}

char const* Parse( Vec3* out, char const* str )
{
	try {
		std::string s( str );
		Strings strings = SplitStringOnDelimiter( s, ',' );
		*out = Vec3( stof( strings[0] ), stof( strings[1] ), stof( strings[2] ) );
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}

char const* Parse( IntVec2* out, char const* str )
{
	try {
		std::string s( str );
		Strings strings = SplitStringOnDelimiter( s, ',' );
		*out = IntVec2( stoi( strings[0] ), stoi( strings[1] ) );
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}

char const* Parse( Rgba8* out, char const* str )
{
	try {
		std::string s( str );
		Strings strings = SplitStringOnDelimiter( s, ',' );
		//auto r =  reinterpret_cast<unsigned char>( strings[0].c_str() );
		unsigned char r = (unsigned char)stoul( strings[0] );
		unsigned char g = (unsigned char)stoul( strings[1] );
		unsigned char b = (unsigned char)stoul( strings[2] );
		unsigned char a = (unsigned char)stoul( strings[3] );
		
		*out = Rgba8( r, g, b, a );
		return str;
	}
	catch( ... ) {
		return nullptr;
	}
}





