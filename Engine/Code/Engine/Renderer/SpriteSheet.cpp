#include "SpriteSheet.hpp"
//#include "SpriteDefinition.hpp"

SpriteSheet::SpriteSheet( const Texture& texture, const IntVec2& simpleGridLayout )
	:m_texture(texture)
	,m_dimensions( simpleGridLayout )
{
	float uEachSpriteGridX = 1.0f / static_cast<float>( simpleGridLayout.x );
	float vEachSpriteGridY = 1.0f / static_cast<float>( simpleGridLayout.y );

	int numOfSprites = simpleGridLayout.x * simpleGridLayout.y;
	const int& spritePerRow = simpleGridLayout.x;

	for( int spriteIndex = 0; spriteIndex < numOfSprites; spriteIndex++ )
	{
		int spriteGridX = spriteIndex % spritePerRow;
		int spriteGridY = spriteIndex / spritePerRow;

		float uAtMinX = uEachSpriteGridX * static_cast<float>( spriteGridX );
		float uAtMaxX = uAtMinX + uEachSpriteGridX;
		float vAtMaxY = 1.0f - vEachSpriteGridY * static_cast<float>( spriteGridY );
		float vAtMinY = vAtMaxY - vEachSpriteGridY;

		Vec2 uvAtMins( uAtMinX, vAtMinY );
		Vec2 uvAtMaxs( uAtMaxX, vAtMaxY );
		m_spriteDefs.push_back( SpriteDefinition( *this, spriteIndex, uvAtMins, uvAtMaxs ) );
	}
}

const SpriteDefinition& SpriteSheet::GetSpriteDefinition( int spriteIndex ) const
{
	return m_spriteDefs[ spriteIndex ];
}

void SpriteSheet::GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const
{
	m_spriteDefs[ spriteIndex ].GetUVs( out_uvAtMins, out_uvAtMaxs );
}

AABB2 SpriteSheet::GetTextureCoordsForSpriteCoords( const IntVec2& spriteCoords ) const 
{
	const int index = spriteCoords.y * m_dimensions.x + spriteCoords.x; 
	SpriteDefinition spriteDef = GetSpriteDefinition( index );
	Vec2 uvAtMins, uvAtMaxs;
	spriteDef.GetUVs( uvAtMins, uvAtMaxs );

	return AABB2( uvAtMins, uvAtMaxs );
}
