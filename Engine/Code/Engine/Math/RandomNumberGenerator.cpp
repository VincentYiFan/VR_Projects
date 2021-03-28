#include "RandomNumberGenerator.hpp"
#include "Engine/Math/RawNoise.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <cstdlib> // remove after we replace rand() with noise



int RandomNumberGenerator::RollRandomIntLessThan( int maxNotInclusive )
{
	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	return randomBits % maxNotInclusive;
	//return rand() % maxNotInclusive;
}

int RandomNumberGenerator::RollRandomIntInRange( int minInclusive, int maxInclusive )
{
	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	return randomBits % ( maxInclusive - minInclusive + 1 ) + minInclusive;
	//return rand() % ( maxInclusive - minInclusive + 1 ) + minInclusive;
}

float RandomNumberGenerator::RollRandomFloatLessThan( float maxNotInclusive )
{
	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	return static_cast<float>( (double) randomBits  / ( (double) 0xFFFFFFFF / (double) maxNotInclusive ) );
	//return static_cast<float>(rand()) / static_cast<float>(RAND_MAX/maxNotInclusive);
}

float RandomNumberGenerator::RollRandomFloatInRange( float minInclusive, float maxInclusive )
{
	if( maxInclusive - minInclusive == 0.f )
		return 0.f;

	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	return minInclusive + static_cast<float>( (double) randomBits / ( (double) 0xFFFFFFFF / ((double) maxInclusive - (double) minInclusive) ) );
	//return minInclusive + static_cast<float>(rand() / static_cast<float>(RAND_MAX / (maxInclusive - minInclusive)));
}

float RandomNumberGenerator::RollRandomFloatZeroToOneInclusive()
{
	constexpr double scale =  1.0 / (double) 0xFFFFFFFF;
	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	return (float)( scale * (double) randomBits );
}

float RandomNumberGenerator::RollRandomFloatZeroToAlmostOne()
{
	constexpr double scale =  1.0 / ( 1.0 + (double) ( 0xFFFFFFFF ) );
	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	return (float)( scale * (double) randomBits );
}

bool RandomNumberGenerator::RollPercentChance( float probabilityOfReturningTrue ) 
{
	constexpr double scale =  1.0 / (1.0 + (double)(0xFFFFFFFF));
	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	return  (float)( randomBits % 100 ) < ( probabilityOfReturningTrue * 100.f );
}

Vec2 RandomNumberGenerator::RollRandomDirection2D()
{
	constexpr double scale =  1.0 / (1.0 + (double)(0xFFFFFFFF));
	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	int directionIndex = (int) ( randomBits % 4 );
	switch( directionIndex )
	{
		case 0: return Vec2( 1.f, 0.f );
		case 1: return Vec2( 0.f, 1.f );
		case 2: return Vec2( -1.f, 0.f );
		case 3: return Vec2( 0.f, -1.f );
		default: ERROR_AND_DIE( "Roll Direction Error!" );
	}
}

IntVec2 RandomNumberGenerator::RollRandomDirectionIntVec2()
{
	constexpr double scale =  1.0 / (1.0 + (double)(0xFFFFFFFF));
	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	int directionIndex = (int)(randomBits % 4);
	switch( directionIndex )
	{
	case 0: return IntVec2( 1, 0 );
	case 1: return IntVec2( 0, 1 );
	case 2: return IntVec2( -1, 0 );
	case 3: return IntVec2( 0, -1 );
	default: ERROR_AND_DIE( "Roll Direction IntVec2 Error!" );
	}
}

void RandomNumberGenerator::Reset( unsigned int seed )
{
	m_seed = seed;
}
