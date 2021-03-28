#pragma once

struct Vec2;
struct IntVec2;

class RandomNumberGenerator 
{
public:
	// Note: these are NON-CONST (mutating) methods, as RNG state is affected by each roll!
	int		 RollRandomIntLessThan		( int maxNotInclusive );
	int		 RollRandomIntInRange		( int minInclusive, int maxInclusive );
	float	 RollRandomFloatLessThan	( float maxNotInclusive );
	float	 RollRandomFloatInRange		( float minInclusive, float maxInclusive );
	float	 RollRandomFloatZeroToOneInclusive();
	float	 RollRandomFloatZeroToAlmostOne();
	bool	 RollPercentChance( float probabilityOfReturningTrue );
	Vec2	 RollRandomDirection2D();
	IntVec2	 RollRandomDirectionIntVec2();

	void	Reset( unsigned int seed = 0 );

private:
	 unsigned int	m_seed = 0; // Will use this later when replacing rand() using noise
	 int			m_position = 0; 
	
};
