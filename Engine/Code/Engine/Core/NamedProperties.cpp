#include "NamedProperties.hpp"


void NamedProperties::PopulateFromEvent( const std::string& commandInputWithValue )
{
	Strings strings = SplitStringOnDelimiter( commandInputWithValue, ' ' );
	
	if( strings.size() <= 1 )
	{
		return;
	}

	SetValue( strings[0], strings[1].c_str() );
}
