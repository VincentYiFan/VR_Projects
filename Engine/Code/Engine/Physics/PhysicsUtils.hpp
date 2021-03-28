#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Physics/Plane2D.hpp"
#include <vector>

//----------------------------------------------------------------------------------------------------------
class PolygonCollider2D;
//----------------------------------------------------------------------------------------------------------

constexpr float TOLERANCE = 0.001f;


enum EvolveResult
{
	NoIntersection,
	FoundIntersection,
	StillEvolving
};


// GJK Intersection
Vec2 SupportPointForMinkowskiDiff( PolygonCollider2D const* shapeA, PolygonCollider2D const* shapeB, Vec2 dir );

EvolveResult EvolveSimplex( std::vector<Vec2>& vertices, Vec2& direction, PolygonCollider2D const* shapeA, PolygonCollider2D const* shapeB );

Vec2 TripleProduct( const Vec2 A, const Vec2 B, const Vec2 C );

bool AddSupport( std::vector<Vec2>& vertices, Vec2& direction, PolygonCollider2D const* shapeA, PolygonCollider2D const* shapeB );

Plane2D FindClosestEdge( std::vector<Vec2> const& simplex );

// clips the line segment points v1, v2 if they are past o along n
std::vector<Vec2> clip( Vec2 v1, Vec2 v2, Vec2 n, float o );