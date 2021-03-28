#pragma once
#include <string>
#include <vector>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"

class Image
{
public:
	Image( const char* imageFilePath );
	const std::string& GetImageFilePath() const;
	IntVec2			GetDimensions() const;
	Rgba8			GetTexelColor( int texelX, int texelY ) const;
	Rgba8			GetTexelColor( const IntVec2& texelCoords ) const;
	void			SetTexelColor( int texelX, int texelY, const Rgba8& newColor );
	void			SetTexelColor( const IntVec2& texelCoords , const Rgba8& newColor );
	void			RotateImage90DegreesCW();

private:
	std::string		m_imageFilePath;
	IntVec2			m_dimensions = IntVec2( 0, 0 );
	std::vector< Rgba8 >	m_rgbaTexels;
};

