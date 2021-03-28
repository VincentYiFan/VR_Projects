#pragma once
#include "Vec2.hpp"
#include <vector>

class ConvexPoly2
{
public:
	bool IsPointInside( const Vec2& point ) const;


private:
	std::vector<Vec2> m_ccwPoints;
};
