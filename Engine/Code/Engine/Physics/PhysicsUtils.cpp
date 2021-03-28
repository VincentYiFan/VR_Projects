#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <string>
#include <limits>

Vec2 SupportPointForMinkowskiDiff( PolygonCollider2D const* shapeA, PolygonCollider2D const* shapeB, Vec2 dir )
{
	/*std::vector<Vec2> pointsA;
	pointsA.push_back( Vec2( 4, 5 ) );
	pointsA.push_back( Vec2( 9, 9 ) );
	pointsA.push_back( Vec2( 4, 11 ) );
	PolygonCollider2D* A = new PolygonCollider2D( pointsA );

	std::vector<Vec2> pointsB;
	pointsB.push_back( Vec2( 10, 2 ) );
	pointsB.push_back( Vec2( 12, 7 ) );
	pointsB.push_back( Vec2( 5, 7 ) );
	pointsB.push_back( Vec2( 7, 3 ) );
	PolygonCollider2D* B = new PolygonCollider2D( pointsB );

	Vec2 a = A->Support( dir );
	Vec2 b = B->Support( -dir );

	return a - b;*/
	return shapeA->Support( dir ) - shapeB->Support( -dir );
}


EvolveResult EvolveSimplex( std::vector<Vec2>& vertices, Vec2& direction, PolygonCollider2D const* shapeA, PolygonCollider2D const* shapeB )
{
	switch( vertices.size() )
	{
		case 0:
		{
			//direction = shapeB->GetCenter() - shapeA->GetCenter();
			direction = shapeB->m_worldPosition - shapeA->m_worldPosition;

			break;
		}

		case 1:
		{
			// flip the direction
			direction = -direction;

			break;
		}

		case 2:
		{
			Vec2 b = vertices[1];
			Vec2 c = vertices[0];

			// line cb is the line formed by the first two vertices
			Vec2 cb = b - c;

			// line c0 is the line from the first vertex to the origin
			Vec2 c0 = Vec2::ZERO - c;

			direction = TripleProduct( cb, c0, cb );
			//vertices.push_back( SupportPointForMinkowskiDiff( shapeA, shapeB, direction ) );

			break;
		}

		case 3:
		{
			// calculate if the simplex contains the origin
			Vec2 a = vertices[2];
			Vec2 b = vertices[1];
			Vec2 c = vertices[0];

			Vec2 a0 = Vec2::ZERO - a;
			Vec2 ab = b - a;
			Vec2 ac = c - a;

			Vec2 abPerp = TripleProduct( ac, ab, ab );
			Vec2 acPerp = TripleProduct( ab, ac, ac );

			if( DotProduct2D( abPerp, a0 ) > 0 )
			{
				// the origin is outside line ab
				// get rid of c and add a new support in the direction of abPerp
				vertices.erase( vertices.begin() + 0 );
				direction = abPerp;
			}
			else if( DotProduct2D( acPerp, a0 ) > 0 )
			{
				// the origin is outside line ac
				// get rid of b and add a new support in the direction of acPerp
				vertices.erase( vertices.begin() + 1 );
				direction = acPerp;
			}
			else
			{
				// the origin is inside both ab and ac
				// so it must be inside the triangle!
				return FoundIntersection;
			}

			break;
		}

		default:
		{
			//std::string text = "Can't have simplex with " + std::to_string( vertices.size() ) + " vertices!";
			//g_theConsole->PrintString( Rgba8::RED, text );
			break;
		}
	}
	return AddSupport( vertices, direction, shapeA, shapeB ) ? StillEvolving : NoIntersection;
}

Vec2 TripleProduct( const Vec2 A, const Vec2 B, const Vec2 C )
{
	//return ( DotProduct2D( B, C ) * -A ) - ( DotProduct2D( A, C ) * B ); 

	Vec3 first = CrossProduct( Vec3( A.x, A.y, 0.f ), Vec3( B.x, B.y, 0.f ) );
	Vec3 second = CrossProduct( first, Vec3( C.x, C.y, 0.f ) );

	return Vec2( second.x, second.y );
}

bool AddSupport( std::vector<Vec2>& vertices, Vec2& direction, PolygonCollider2D const* shapeA, PolygonCollider2D const* shapeB )
{
	Vec2 newSupportPoint =  SupportPointForMinkowskiDiff( shapeA, shapeB, direction );
	vertices.push_back( newSupportPoint );

	return DotProduct2D( direction, newSupportPoint ) > 0;
}

Plane2D FindClosestEdge( std::vector<Vec2> const& simplex )
{
	Plane2D closestEdge;
	closestEdge.distanceFromOriginAlongNormal = std::numeric_limits<float>::infinity();

	for( int i = 0; i < (int)simplex.size(); ++i )
	{
		int j = ( i + 1 ) % (int)simplex.size();
		// int j = i + 1 == (int)simplex.size() ? 0 : i + 1		// another way to express
		
		Vec2 a = simplex[i];
		Vec2 b = simplex[j];

		// create the edge vector
		Vec2 e = b - a;
		if( e.GetLengthSquared() == 0.f ) {
			continue;
		}

		// get the vector from the origin to a
		Vec2 oa = a;	// or a - ORIGIN

		// get the vector from the edge towards the origin
		Vec2 n = TripleProduct( e, oa, e );

		if( n.GetLengthSquared() == 0.f ) {
			n.x = e.y;
			n.y = -e.x;
		}

		// normalize the vector
		n.Normalize();

		// calculate the distance from the origin to the edge
		float d = fabsf( DotProduct2D( n, a ) ); // could use b or a here

		// check the distance against the other distances
		if( d < closestEdge.distanceFromOriginAlongNormal ) {
			// if this edge is closer then use it
			closestEdge.distanceFromOriginAlongNormal = d;
			closestEdge.normal = n;
			closestEdge.index = j;
		}
	}

	return closestEdge;
}

std::vector<Vec2> clip( Vec2 v1, Vec2 v2, Vec2 n, float o )
{
	std::vector<Vec2> cp;
	float d1 = DotProduct2D( n, v1 ) - o;
	float d2 = DotProduct2D( n, v2 ) - o;
	// if either point is past o along n
	// then we can keep the point
	if( d1 >= 0.0 ) cp.push_back( v1 );
	if( d2 >= 0.0 ) cp.push_back( v2 );
	// finally we need to check if they
	// are on opposing sides so that we can
	// compute the correct point
	if( d1 * d2 < 0.f ) {
		// if they are on different sides of the
		// offset, d1 and d2 will be a (+) * (-)
		// and will yield a (-) and therefore be
		// less than zero
		// get the vector for the edge we are clipping
		Vec2 e = v2 - v1;
		// compute the location along e
		float u = d1 / (d1 - d2);
		e *= u;
		e += v1;
		// add the point
		cp.push_back( e );
	}

	return cp;
}
