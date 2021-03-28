#include "BitmapFont.hpp"
#include "SpriteDefinition.hpp"
#include "SpriteSheet.hpp"
#include "Engine/Math/AABB2.hpp"

BitmapFont::BitmapFont( const char* fontName, const Texture* fontTexture )
	:m_fontName( fontName )
	,m_glyphSpriteSheet( SpriteSheet( *fontTexture, IntVec2( 16, 16 ) ) )
{

}

const Texture* BitmapFont::GetTexture() const
{
	return &m_glyphSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForText2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect )
{
	for( int i = 0; i < text.length(); i++ )
	{
		char c = text[i];
		float cellWidth = cellHeight * GetGlyphAspect( c ) * cellAspect;
		SpriteDefinition spriteDef = m_glyphSpriteSheet.GetSpriteDefinition( c );

		Vec2 uvMins;
		Vec2 uvMaxs;
		spriteDef.GetUVs( uvMins, uvMaxs );

		Vec2 theTextMins = textMins + Vec2( cellWidth, 0.f ) * (float)i;
		Vec2 theTextMaxs = theTextMins + Vec2( cellWidth, cellHeight );

		vertexArray.push_back( Vertex_PCU( Vec3( theTextMaxs.x, theTextMins.y, 0.f ), tint, Vec2( uvMaxs.x, uvMins.y ) ) );
		vertexArray.push_back( Vertex_PCU( Vec3( theTextMaxs.x, theTextMaxs.y, 0.f ), tint, Vec2( uvMaxs.x, uvMaxs.y ) ) );
		vertexArray.push_back( Vertex_PCU( Vec3( theTextMins.x, theTextMins.y, 0.f ), tint, Vec2( uvMins.x, uvMins.y ) ) );

		vertexArray.push_back( Vertex_PCU( Vec3( theTextMaxs.x, theTextMaxs.y, 0.f ), tint, Vec2( uvMaxs.x, uvMaxs.y ) ) );
		vertexArray.push_back( Vertex_PCU( Vec3( theTextMins.x, theTextMaxs.y, 0.f ), tint, Vec2( uvMins.x, uvMaxs.y ) ) );
		vertexArray.push_back( Vertex_PCU( Vec3( theTextMins.x, theTextMins.y, 0.f ), tint, Vec2( uvMins.x, uvMins.y ) ) );
	}
}

void BitmapFont::AddVertsForTextInBox2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& box, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect, const Vec2& alignment )
{
	// Assume all text can fit inside the box
	Vec2 textDimensions = GetDimensionsForText2D( cellHeight, text, cellAspect );
	Vec2 textMins = Vec2::ZERO;
	Vec2 boxDimensions = box.GetDimensions();

	//if( alignment.x == 0.f ){ textMins.x = box.mins.x; }
	//else if( alignment.x == 0.5f ){ textMins.x = box.GetCenter().x - (0.5f * textDimensions.x); } // ToDo: Change it to else than 0&1, that means every Vec2 works. Check If the x, y of Vec2 is within 0~1
	//else if( alignment.x == 1.0f ){ textMins.x = box.maxs.x - textDimensions.x; }

	//if( alignment.y == 0.f ){ textMins.y = box.mins.y; }
	//else if( alignment.y == 0.5f ){ textMins.y = box.GetCenter().y - (0.5f * textDimensions.y); }
	//else if( alignment.y == 1.0f ){ textMins.y = box.maxs.y - textDimensions.y; }

	Vec2 remainDimensions = boxDimensions - textDimensions;
	textMins = box.mins + Vec2( remainDimensions.x * alignment.x, remainDimensions.y * alignment.y );

	AddVertsForText2D(vertexArray, textMins, cellHeight, text, tint, cellAspect );
}

Vec2 BitmapFont::GetDimensionsForText2D( float cellHeight, const std::string& text, float cellAspect )
{
	float sumOfWidths = 0.f;

	for( int index = 0; index < text.length(); index++ )
	{
		char c = text[index];
		float cellWidth = cellHeight * GetGlyphAspect( c ) * cellAspect;
		sumOfWidths += cellWidth;
	}

	return Vec2( sumOfWidths, cellHeight );
}

void BitmapFont::AddVertsForText3D( std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float z, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect )
{
	for( int i = 0; i < text.length(); i++ )
	{
		char c = text[i];
		float cellWidth = cellHeight * GetGlyphAspect( c ) * cellAspect;
		SpriteDefinition spriteDef = m_glyphSpriteSheet.GetSpriteDefinition( c );

		Vec2 uvMins;
		Vec2 uvMaxs;
		spriteDef.GetUVs( uvMins, uvMaxs );

		Vec2 theTextMins = textMins + Vec2( cellWidth, 0.f ) * (float)i;
		Vec2 theTextMaxs = theTextMins + Vec2( cellWidth, cellHeight );

		vertexArray.push_back( Vertex_PCU( Vec3( theTextMaxs.x, theTextMins.y, z ), tint, Vec2( uvMaxs.x, uvMins.y ) ) );
		vertexArray.push_back( Vertex_PCU( Vec3( theTextMaxs.x, theTextMaxs.y, z ), tint, Vec2( uvMaxs.x, uvMaxs.y ) ) );
		vertexArray.push_back( Vertex_PCU( Vec3( theTextMins.x, theTextMins.y, z ), tint, Vec2( uvMins.x, uvMins.y ) ) );
		
		vertexArray.push_back( Vertex_PCU( Vec3( theTextMaxs.x, theTextMaxs.y, z ), tint, Vec2( uvMaxs.x, uvMaxs.y ) ) );
		vertexArray.push_back( Vertex_PCU( Vec3( theTextMins.x, theTextMaxs.y, z ), tint, Vec2( uvMins.x, uvMaxs.y ) ) );
		vertexArray.push_back( Vertex_PCU( Vec3( theTextMins.x, theTextMins.y, z ), tint, Vec2( uvMins.x, uvMins.y ) ) );
	}
}

float BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	UNUSED( glyphUnicode );
	return 1.f;
}
