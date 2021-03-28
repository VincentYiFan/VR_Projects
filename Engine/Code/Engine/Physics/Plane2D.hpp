#pragma once
#include "Engine/Math/Vec2.hpp"

struct Plane2D
{
	Vec2 normal;
	float distanceFromOriginAlongNormal; // distance; 

	// For EPA
	int index;

	float GetDistance( Vec2 position ) const;
};
