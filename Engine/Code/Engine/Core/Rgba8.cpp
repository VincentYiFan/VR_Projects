#include "Engine/Core/Rgba8.hpp"
#include <Engine\Core\StringUtils.hpp>

const Rgba8 Rgba8::WHITE( 255, 255, 255, 255 );
const Rgba8 Rgba8::BLACK( 0, 0, 0, 255 );
const Rgba8 Rgba8::RED( 255, 0, 0, 255 );
const Rgba8 Rgba8::GREEN( 0, 255, 0, 255 );
const Rgba8 Rgba8::BLUE( 0, 0, 255, 255 );
const Rgba8 Rgba8::YELLOW( 255, 255, 0, 255 );
const Rgba8 Rgba8::CYAN( 0, 255, 255, 255 );
const Rgba8 Rgba8::MAGENTA( 255, 0, 255, 255 );
const Rgba8 Rgba8::GREY( 105, 105, 105, 150 );

Rgba8::Rgba8(  unsigned char initialR, unsigned char initialG, unsigned char initialB, unsigned char initialA )  //just define once
	: r(initialR)
	, g(initialG)
	, b(initialB)
	, a(initialA)
{
}

Rgba8::~Rgba8() = default;

void Rgba8::SetFromText( const char* text )
{
	Strings strings = SplitStringOnDelimiter( text, ',' );

	const char* rValueChar  = strings[0].c_str();
	const char* gValueChar = strings[1].c_str();
	const char* bValueChar = strings[2].c_str();
	const char* aValueChar = nullptr;
	if( strings.size() >= 4 ){ 
		aValueChar = strings[3].c_str();
	}

	r =  (unsigned char)atoi( rValueChar );
	g =  (unsigned char)atoi( gValueChar );
	b =  (unsigned char)atoi( bValueChar );
	if( strings.size() >= 4 )
	{
		a = (unsigned char) atoi( aValueChar ); 
	}
	else 
	{
		a = (unsigned char) 255; 
	}
}

bool Rgba8::operator==( const Rgba8& compare ) const
{
	return ( r == compare.r 
		  && g == compare.g 
		  && b == compare.b 
		  && a == compare.a );
}

void Rgba8::operator=( const Rgba8& copyFrom )
{
	r = copyFrom.r;
	g = copyFrom.g;
	b = copyFrom.b;
	a = copyFrom.a;
}