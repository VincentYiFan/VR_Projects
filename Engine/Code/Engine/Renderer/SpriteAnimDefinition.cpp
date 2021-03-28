#include "SpriteAnimDefinition.hpp"
#include <math.h>
#include <algorithm>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


SpriteAnimDefinition::SpriteAnimDefinition( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType )
	:m_spriteSheet( sheet ),
	m_durationSeconds( durationSeconds ),
	m_playbackType( playbackType )
{
	if( startSpriteIndex < endSpriteIndex )
	{
		for( int spriteIndex = startSpriteIndex; spriteIndex <= endSpriteIndex; spriteIndex++ )
		{
			m_spriteIndexes.push_back( spriteIndex );
		}
	}
	else 
	{
		for( int spriteIndex = startSpriteIndex; spriteIndex >= endSpriteIndex; spriteIndex-- )
		{
			m_spriteIndexes.push_back( spriteIndex );
		}
	}
}
SpriteAnimDefinition::SpriteAnimDefinition( const SpriteSheet& sheet, const std::vector<int>& spriteIndexes, float durationSeconds, SpriteAnimPlaybackType playbackType )
	:m_spriteSheet( sheet ),
	m_durationSeconds( durationSeconds ),
	m_playbackType( playbackType )
{
	GUARANTEE_OR_DIE( !spriteIndexes.empty(), "Invalid spriteIndexes vector");
	m_spriteIndexes = spriteIndexes;
}

const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTime( float seconds ) const
{
	if( m_playbackType == SpriteAnimPlaybackType::ONCE )
	{ 
		int numFrames = (int) m_spriteIndexes.size();
		float perSpriteDurationSeconds = m_durationSeconds / numFrames;
		int currentFrame = static_cast<int>( Clamp( (int)floor( seconds / perSpriteDurationSeconds ), 0, numFrames - 1 ) ); 
		int SpriteIndex = m_spriteIndexes[ currentFrame ];

		return m_spriteSheet.GetSpriteDefinition( SpriteIndex );
	}
	else if( m_playbackType == SpriteAnimPlaybackType::LOOP )
	{
		int numFrames = (int) m_spriteIndexes.size();
		float perSpriteDurationSeconds = m_durationSeconds / numFrames;
		int frameIndex =  (int)floor( seconds / perSpriteDurationSeconds );
		frameIndex = frameIndex % numFrames;
		while( frameIndex < 0 )
		{
			frameIndex += numFrames;
		}
		int spriteIndex = m_spriteIndexes[ frameIndex ];
		return  m_spriteSheet.GetSpriteDefinition( spriteIndex );
	}
	else if( m_playbackType == SpriteAnimPlaybackType::PINGPONG )
	{
		int numFrames = ( 2 * (int) m_spriteIndexes.size() ) -2;
		float perSpriteDurationSeconds = m_durationSeconds / numFrames;
		int currentFrame = (int)floor(  seconds / perSpriteDurationSeconds );
		int frameIndex = currentFrame % numFrames;
		int spriteIndexOffest = frameIndex;
		if( frameIndex > (numFrames * 0.5f) )
		{
			spriteIndexOffest = numFrames - frameIndex;
		}
		int spriteIndex = m_spriteIndexes[ spriteIndexOffest ]; // need to check if correct

		return  m_spriteSheet.GetSpriteDefinition( spriteIndex );
	}

	else
	{
		ERROR_AND_DIE( Stringf( "Unknown sprite animation playback type #%i", m_playbackType ) );
	}

}
