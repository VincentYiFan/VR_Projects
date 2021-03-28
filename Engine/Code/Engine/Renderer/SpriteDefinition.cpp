#include "SpriteDefinition.hpp"
#include "SpriteSheet.hpp"

SpriteDefinition::SpriteDefinition( const SpriteSheet& spriteSheet, int spriteIndex, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
	:m_spriteSheet(spriteSheet)
	,m_spriteIndex(spriteIndex)
	,m_uvAtMins(uvAtMins)
	,m_uvAtMaxs(uvAtMaxs)
{
}

void SpriteDefinition::GetUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs ) const
{
	out_uvAtMins = m_uvAtMins;
	out_uvAtMaxs = m_uvAtMaxs;
}

const SpriteSheet SpriteDefinition::GetSpriteSheet() const
{
	return m_spriteSheet;
}

const Texture& SpriteDefinition::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}

float SpriteDefinition::GetAspect() const
{
	float theTextureAspect = m_spriteSheet.GetTexture().GetAspect();
	float theuvWidth  = (float)fabs( m_uvAtMaxs.x - m_uvAtMins.x );
	float theuvHeight = (float)fabs( m_uvAtMaxs.y - m_uvAtMins.y );
	
	return theTextureAspect * ( theuvWidth / theuvHeight );
}
