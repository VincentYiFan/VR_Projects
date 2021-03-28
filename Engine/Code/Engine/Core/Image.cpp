#include "Image.hpp"
#include "Engine/Renderer/stb_image.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <string>
#include <vector>
#include <math.h> 

Image::Image( const char* imageFilePath )
{
	m_imageFilePath = imageFilePath;

	//const char* imageFilePath = imageFilePath;
	//unsigned int textureID = 0;
	int imageTexelSizeX = 0; // This will be filled in for us to indicate image width
	int imageTexelSizeY = 0; // This will be filled in for us to indicate image height
	int numComponents = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

									// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* imageData = stbi_load( imageFilePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );


	std::string imageDataString( ( char* )imageData );
	std::vector<char> imageDataVector( imageDataString.begin(), imageDataString.end() );

	// Check if the load was successful
	GUARANTEE_OR_DIE( imageData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );
	GUARANTEE_OR_DIE( numComponents >= 3 && numComponents <= 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, imageTexelSizeX, imageTexelSizeY ) );

	m_dimensions = IntVec2( imageTexelSizeX, imageTexelSizeY );
	int numTexels = m_dimensions.x * m_dimensions.y;

	if( numComponents == 3 )
	{
		for( int index = 0; index < 3 * numTexels; index += 3 )
		{
			m_rgbaTexels.push_back( Rgba8( imageData[index], imageData[index + 1], imageData[index + 2], 255 ) );
		}
	}
	else if( numComponents == 4 )
	{
		for( int index = 0; index < 4 * numTexels; index += 4 )
		{
			m_rgbaTexels.push_back( Rgba8( imageData[index], imageData[index + 1], imageData[index + 2], imageData[index + 3] ) );
		}
	}

	stbi_image_free( imageData );
}

const std::string& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

Rgba8 Image::GetTexelColor( int texelX, int texelY ) const
{
	int texelIndex = ( m_dimensions.x * texelY ) + texelX;
	return m_rgbaTexels[texelIndex];
}

Rgba8 Image::GetTexelColor( const IntVec2& texelCoords ) const
{
	int texelIndex = ( m_dimensions.x * texelCoords.y ) + texelCoords.x;
	return m_rgbaTexels[texelIndex];
}

void Image::SetTexelColor( int texelX, int texelY, const Rgba8& newColor )
{
	int texelIndex = ( m_dimensions.x * texelY ) + texelX;
	m_rgbaTexels[texelIndex] = newColor;
}

void Image::SetTexelColor( const IntVec2& texelCoords, const Rgba8& newColor )
{
	int texelIndex = ( m_dimensions.x * texelCoords.y ) + texelCoords.x;
	m_rgbaTexels[texelIndex] = newColor;
}

void Image::RotateImage90DegreesCW(/* std::vector<Rgba8>& inputVector, IntVec2& dimensions */)
{
	std::vector<Rgba8> newVector( m_rgbaTexels );

	for( int j = 0; j < m_dimensions.y; j++ )
	{
		for( int i = 0; i < m_dimensions.x; i++ )
		{
			int newX = j;
			int newY = m_dimensions.x - 1 - i;

			IntVec2 newDimensions = IntVec2( m_dimensions.y, m_dimensions.x );
			int newIndex = newY * newDimensions.x + newX;
			int oldIndex = j * m_dimensions.x + i;

			newVector[ newIndex ] = m_rgbaTexels[ oldIndex ];
		}
	}

	m_dimensions = IntVec2( m_dimensions.y, m_dimensions.x );
	m_rgbaTexels.assign( newVector.begin(), newVector.end() );
}
