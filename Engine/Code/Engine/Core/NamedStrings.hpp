#pragma once
#include "XmlUtils.hpp"
#include <map>

class NamedStrings 
{
public:
	void			PopulateFromXmlElementAttributes( const XmlElement& element );
	bool			PopulateFromXmlFile( const char* xmlFilePath );
	void			SetValue( const std::string& keyName, const std::string& newValue );

	bool			GetValue( const std::string& keyName, bool defaultValue ) const;
	int				GetValue( const std::string& keyName, int defaultValue ) const;
	float			GetValue( const std::string& keyName, float defaultValue ) const;
	std::string		GetValue( const std::string& keyName, std::string defaultValue ) const;
	std::string		GetValue( const std::string& keyName, const char* defaultValue ) const;
	Rgba8			GetValue( const std::string& keyName, const Rgba8& defaultValue ) const;
	Vec2			GetValue( const std::string& keyName, const Vec2& defaultValue ) const;
	IntVec2			GetValue( const std::string& keyName, const IntVec2& defaultValue ) const;
	
	static NamedStrings PopulateFromEvent( const std::string& commandInputWithValue );


private:
	std::map< std::string, std::string >	m_keyValuePairs;
};
