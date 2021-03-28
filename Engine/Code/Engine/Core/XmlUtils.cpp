#include "XmlUtils.hpp"

int ParseXmlAttribute( const XmlElement& element, const char* attributeName, int defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	int value = defaultValue;
	if( attributeValueText )
	{
		value = atoi( attributeValueText );
	}

	return value;
}

uint ParseXmlAttribute( const XmlElement& element, const char* attributeName, uint defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	uint value = defaultValue;
	if( attributeValueText )
	{
		value = uint( atoi( attributeValueText ) );
	}

	return value;
}

char ParseXmlAttribute( const XmlElement& element, const char* attributeName, char defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	char value = defaultValue;
	if( attributeValueText )
	{
		value = attributeValueText[0];
	}

	return value;
}

bool ParseXmlAttribute( const XmlElement& element, const char* attributeName, bool defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	bool value = defaultValue;
	if( attributeValueText )
	{
		std::string s( attributeValueText );
		if( s == "true" )
		{
			value = true;
		}
		else if( s == "false" )
		{
			value = false;
		}
	}

	return value;
}

float ParseXmlAttribute( const XmlElement& element, const char* attributeName, float defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	float value = defaultValue;
	if( attributeValueText )
	{
		value = (float)atof( attributeValueText );
	}

	return value;
}

Rgba8 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Rgba8& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Rgba8 value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}

	return value;
}

Vec2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Vec2& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Vec2 value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}

	return value;
}

Vec3 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Vec3& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Vec3 value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}

	return value;
}

IntVec2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const IntVec2& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	IntVec2 value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}

	return value;
}

IntRange ParseXmlAttribute( const XmlElement& element, const char* attributeName, const IntRange defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	IntRange value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}

	return value;
}

FloatRange ParseXmlAttribute( const XmlElement& element, const char* attributeName, const FloatRange defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	FloatRange value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}

	return value;
}

std::string ParseXmlAttribute( const XmlElement& element, const char* attributeName, const std::string& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	std::string value = defaultValue;
	if( attributeValueText )
	{
		value = attributeValueText;
	}

	return value;
}

std::string ParseXmlAttribute( const XmlElement& element, const char* attributeName, const char* defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	std::string value = defaultValue;
	if( attributeValueText )
	{
		value = attributeValueText;
	}

	return value;
}

Strings ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Strings& defaultValues, const unsigned char delimeter )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Strings value = defaultValues;
	if( attributeValueText )
	{
		value = SplitStringOnDelimiter( attributeValueText, delimeter );
	}

	return value;
}
