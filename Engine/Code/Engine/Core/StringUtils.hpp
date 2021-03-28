#pragma once
#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
// type def
typedef unsigned int uint;
//-----------------------------------------------------------------------------------------------
// forward declaration
struct Vec2;
struct Vec3;
struct IntVec2;
struct Rgba8;
//-----------------------------------------------------------------------------------------------

bool ToBool( std::string str );

const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );

typedef std::vector< std::string >		Strings;

Strings SplitStringOnDelimiter( const std::string& originalString, char delimiterToSplitOn );

//-----------------------------------------------------------------------------------------------
// Get tail of string after first token and possibly following spaces
std::string tail( const std::string& in );

// Get first token of string
std::string firstToken(const std::string &in);

void split( const std::string& in, std::vector<std::string>& out, std::string token );

// Get element at given index position
template <class T>
inline const T& getElement( const std::vector<T>& elements, std::string& index )
{
	int idx = std::stoi( index );
	if( idx < 0 )
		idx = int( elements.size() ) + idx;
	else
		idx--;
	return elements[idx];
}

//-----------------------------------------------------------------------------------------------
// string parsing
//-----------------------------------------------------------------------------------------------
char const* Parse( float* out, char const* str );
char const* Parse( uint* out, char const* str );
char const* Parse( int* out, char const* str );
char const* Parse( double* out, char const* str );
char const* Parse( bool* out, char const* str );
char const* Parse( std::string* out, char const* str );
char const* Parse( Vec2* out, char const* str );
char const* Parse( Vec3* out, char const* str );
char const* Parse( IntVec2* out, char const* str );
char const* Parse( Rgba8* out, char const* str );
//-----------------------------------------------------------------------------------------------

std::string ToString( float value );
std::string ToString( uint value );
std::string ToString( int value );
std::string ToString( double value );
std::string ToString( bool value );
std::string ToString( std::string value );
std::string ToString( Vec2 value );
std::string ToString( Vec3 value );
std::string ToString( IntVec2 value );
std::string ToString( Rgba8 value );

// most objects should have a parse method if they are supported
template <typename T>
inline char const* Parse( T* out, char const* str )
{
	return T::Parse( out, str );;
}

//-----------------------------------------------------------------------------------------------
// try to parse, and if it fails, just return the default value
template<typename T>
T GetValueFromString( char const* str, T const& defValue ) 
{
	T out;
	char const* adv = Parse( &out, str );
	if( adv == nullptr ) {
		return defValue;
	}
	else {
		return out;
	}
}
//-----------------------------------------------------------------------------------------------

template<typename T>
std::string ToString( T value )
{
	return T::ToString( value );
}
//-----------------------------------------------------------------------------------------------




