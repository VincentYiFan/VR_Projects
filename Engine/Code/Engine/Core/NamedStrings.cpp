#include "NamedStrings.hpp"
#include "XmlUtils.hpp"

void NamedStrings::PopulateFromXmlElementAttributes( const XmlElement& element )
{
	const XmlAttribute* pAttr = element.FirstAttribute();
	while ( pAttr )
	{
		const char* name = pAttr->Name();
		const char* value = pAttr->Value();

		m_keyValuePairs[ name ] = value;

		pAttr = pAttr->Next();

	}
}

bool NamedStrings::PopulateFromXmlFile( const char* xmlFilePath )
{
	XmlDocument xmlDocument;
	xmlDocument.LoadFile( xmlFilePath );
	if( xmlDocument.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		return false;
	}

	XmlElement* rootElement = xmlDocument.RootElement();
	if( !rootElement )
	{
		return false;
	}

	PopulateFromXmlElementAttributes( *rootElement );
	return true;
}

void NamedStrings::SetValue( const std::string& keyName, const std::string& newValue )
{
	m_keyValuePairs[ keyName ] = newValue;
}

bool NamedStrings::GetValue( const std::string& keyName, bool defaultValue ) const
{
	bool value = defaultValue;

	if( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		// keyName found!
		const std::string valueString = m_keyValuePairs.at( keyName );
		if( valueString == "true" )
		{
			value = true;
		}
		else if( valueString == "false" )
		{
			value = false;
		}
	}

	return value;
}

int NamedStrings::GetValue( const std::string& keyName, int defaultValue ) const
{
	int value = defaultValue;

	if( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		const std::string valueString = m_keyValuePairs.at( keyName );
		value = stoi( valueString );
	}

	return value;
}

float NamedStrings::GetValue( const std::string& keyName, float defaultValue ) const
{
	float value = defaultValue;

	if( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		const std::string valueString = m_keyValuePairs.at( keyName );
		value = stof( valueString );
	}

	return value;
}

std::string NamedStrings::GetValue( const std::string& keyName, std::string defaultValue ) const
{
	std::string value = defaultValue;

	if( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		const std::string valueString = m_keyValuePairs.at( keyName );
		value = valueString;
	}

	return value;
}

std::string NamedStrings::GetValue( const std::string& keyName, const char* defaultValue ) const
{
	std::string value = defaultValue;

	if( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		const std::string valueString = m_keyValuePairs.at( keyName );
		value = valueString;
	}

	return value;
}

Rgba8 NamedStrings::GetValue( const std::string& keyName, const Rgba8& defaultValue ) const
{
	Rgba8 value = defaultValue;

	if( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		const std::string valueString = m_keyValuePairs.at( keyName );
		value.SetFromText( valueString.c_str() );
	}

	return value;
}

Vec2 NamedStrings::GetValue( const std::string& keyName, const Vec2& defaultValue ) const
{
	Vec2 value = defaultValue;

	if( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		const std::string valueString = m_keyValuePairs.at( keyName );
		value.SetFromText( valueString.c_str() );
	}

	return value;
}

IntVec2 NamedStrings::GetValue( const std::string& keyName, const IntVec2& defaultValue ) const
{
	IntVec2 value = defaultValue;

	if( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		const std::string valueString = m_keyValuePairs.at( keyName );
		value.SetFromText( valueString.c_str() );
	}

	return value;
}

NamedStrings NamedStrings::PopulateFromEvent( const std::string& commandInputWithValue )
{
	Strings strings = SplitStringOnDelimiter( commandInputWithValue, ' ' );

	NamedStrings namedString;
	namedString.m_keyValuePairs[strings[0]] = strings[1];

	return namedString;
}
