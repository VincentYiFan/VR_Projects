#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

class SpriteSheet
{
public:
	explicit SpriteSheet( const Texture& texture, const IntVec2& simpleGridLayout );

	const Texture&				GetTexture() const		{ return m_texture; }
	int							GetNumSprites() const	{ return (int) m_spriteDefs.size(); }
	const SpriteDefinition&		GetSpriteDefinition( int spriteIndex ) const;
	void						GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const;
	AABB2						GetTextureCoordsForSpriteCoords( const IntVec2& spriteCoords ) const;

protected:
	const Texture&					m_texture; // must be set in constructor initializer list
	IntVec2  m_dimensions = IntVec2::ZERO;
	std::vector<SpriteDefinition>	m_spriteDefs;
};
