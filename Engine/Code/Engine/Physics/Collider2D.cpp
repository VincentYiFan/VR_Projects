#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/LineSegment.hpp"
#include "Engine/Physics/Edge.hpp"
#include "Engine/Physics/Plane2D.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include <algorithm>
#include <string>

typedef bool (*collision_check_cb)(Collider2D const*, Collider2D const*);
typedef Manifold2( *manifold_cb )(Collider2D const*, Collider2D const*);



static bool DiscVsDiscCollisionCheck( Collider2D const* col0, Collider2D const* col1 )
{
	DiscCollider2D const* disc0 = (DiscCollider2D const*)col0;
	DiscCollider2D const* disc1 = (DiscCollider2D const*)col1;

	return ( GetDistanceSquared2D(disc0->m_worldPosition, disc1->m_worldPosition) < (disc0->m_radius + disc1->m_radius) * (disc0->m_radius + disc1->m_radius) );
}

static bool DiscVsPolygonCollisionCheck( Collider2D const* col0, Collider2D const* col1 )
{
	DiscCollider2D const* disc = (DiscCollider2D const*)col0;
	PolygonCollider2D const* polygon = (PolygonCollider2D const*)col1; 

	std::vector<Vec2> ringPoints;
	constexpr float numSlices = 72.f;
	const float perSliceDegrees = 360.f / numSlices;
	Vec2 DispForDrawingHalfCircle = Vec2( 1.f, 0.f ) * disc->m_radius;
	float currentDegrees = 0.f;
	while( currentDegrees < 360.f ) {
		ringPoints.push_back( disc->m_worldPosition + DispForDrawingHalfCircle );
		DispForDrawingHalfCircle.RotateDegrees( perSliceDegrees );
		currentDegrees += perSliceDegrees;
	}

	bool hasIntersect = false;
	for( int index = 0; index < (int)ringPoints.size(); index++ )
	{
		if( polygon->Contains( ringPoints[index] ) )
		{
			hasIntersect = true;
		}
	}

	return hasIntersect;
}

static bool PolygonVsDiscCollisionCheck( Collider2D const* col0, Collider2D const* col1 )
{
	return DiscVsPolygonCollisionCheck( col1, col0 );
}

static bool PolygonVsPolygonCollisionCheck( Collider2D const* col0, Collider2D const* col1 )
{
	PolygonCollider2D const* polygonA = (PolygonCollider2D const*)col0; 
	PolygonCollider2D const* polygonB = (PolygonCollider2D const*)col1; 

	std::vector<Vec2> vertices;
	Vec2 direction;

	EvolveResult result = StillEvolving;
	while( result == StillEvolving ) {
		result = EvolveSimplex( vertices, direction, polygonA, polygonB );
	}

	return ( result == FoundIntersection );
}

static Manifold2 DiscVsDiscCollisionManifold( Collider2D const* col0, Collider2D const* col1 )
{
	Manifold2 mf;

	DiscCollider2D const* disc0 = (DiscCollider2D const*)col0;
	DiscCollider2D const* disc1 = (DiscCollider2D const*)col1;

	mf.m_normal = (disc0->m_worldPosition - disc1->m_worldPosition).GetNormalized();
	mf.m_peneration = ( disc0->m_radius + disc1->m_radius ) - GetDistance2D( disc0->m_worldPosition, disc1->m_worldPosition );
	return mf;
}

static Manifold2 DiscVsPolygonCollisionManifold( Collider2D const* col0, Collider2D const* col1 )
{
	Manifold2 mf;

	DiscCollider2D const* disc = (DiscCollider2D const*)col0;
	PolygonCollider2D const* polygon = (PolygonCollider2D const*)col1; 

	Vec2 nearestPointOnPolygon = polygon->GetClosestPoint( disc->m_worldPosition );
	Vec2 normal = disc->m_worldPosition - nearestPointOnPolygon;
	normal.Normalize();
	float peneration = 0.f;

	// Edge case
	if( disc->m_worldPosition == nearestPointOnPolygon ) {
		nearestPointOnPolygon += Vec2( 0.001f, 0.001f );
	}

	float d = GetDistance2D( disc->m_worldPosition, nearestPointOnPolygon );
	
	if( polygon->Contains( disc->m_worldPosition ) ) {
		normal = -normal;
		peneration = disc->m_radius + d;
	}
	else {
		peneration = disc->m_radius - d;
	}

	mf.m_normal = normal;
	mf.m_peneration = peneration;
	return mf;
}

static Manifold2 PolygonVsDiscCollisionManifold( Collider2D const* col0, Collider2D const* col1 )
{
	Manifold2 mf = DiscVsPolygonCollisionManifold( col1, col0 );
	mf.m_normal = -mf.m_normal;
	return mf;
}

static Manifold2 PolygonVsPolygonCollisionManifold( Collider2D const* col0, Collider2D const* col1 )
{
	// create a manifold
	Manifold2 mf;

	PolygonCollider2D const* polygonA = (PolygonCollider2D const*)col0;
	PolygonCollider2D const* polygonB = (PolygonCollider2D const*)col1;

	std::vector<Vec2> simplex;
	Vec2 direction;

	// For testing different cases

	//std::vector<Vec2> pointsA;
	// example 1
	/*pointsA.push_back( Vec2( 6, 4 ) );
	pointsA.push_back( Vec2( 9, 7 ) );
	pointsA.push_back( Vec2( 5, 11 ) );
	pointsA.push_back( Vec2( 2, 8 ) );*/
	// example 2
	/*pointsA.push_back( Vec2( 8, 4 ) );
	pointsA.push_back( Vec2( 14, 4 ) );
	pointsA.push_back( Vec2( 14, 9 ) );
	pointsA.push_back( Vec2( 8, 9 ) );*/
	// example 3
	/*pointsA.push_back( Vec2( 9, 4 ) );
	pointsA.push_back( Vec2( 13, 3 ) );
	pointsA.push_back( Vec2( 14, 7 ) );
	pointsA.push_back( Vec2( 10, 8 ) );
	PolygonCollider2D* A = new PolygonCollider2D( pointsA );
	A->m_worldPosition = polygonA->m_worldPosition;

	std::vector<Vec2> pointsB;
	pointsB.push_back( Vec2( 4, 2 ) );
	pointsB.push_back( Vec2( 12, 2 ) );
	pointsB.push_back( Vec2( 12, 5 ) );
	pointsB.push_back( Vec2( 4, 5 ) );
	PolygonCollider2D* B = new PolygonCollider2D( pointsB );
	B->m_worldPosition = polygonB->m_worldPosition;

	polygonA = A;
	polygonB = B;*/


	EvolveResult result = StillEvolving;
	while( result == StillEvolving ) {
		result = EvolveSimplex( simplex, direction, polygonA, polygonB );
	}

	if( result == FoundIntersection )
	{
		// loop to find the collision information
		while( true ) {
			// obtain the feature (edge for 2D) closest to the 
			// origin on the Minkowski Difference
			Plane2D edge = FindClosestEdge( simplex );


			// obtain a new support point in the direction of the edge normal
			Vec2 p = SupportPointForMinkowskiDiff( polygonA, polygonB, edge.normal );


			// check the distance from the origin to the edge against the
			// distance p is along e.normal
			float d = DotProduct2D( p, edge.normal );
			if( d - edge.distanceFromOriginAlongNormal < TOLERANCE ) {
				// the tolerance should be something positive close to zero (ex. 0.00001)

				// if the difference is less than the tolerance then we can
				// assume that we cannot expand the simplex any further and
				// we have our solution
				mf.m_normal = -edge.normal;
				mf.m_peneration = d;

				// find contact point:
				// find the "best" edge for shape A
				Edge e1 = polygonA->BestEdge( edge.normal ); 
				// find the "best" edge for shape B
				Edge e2 = polygonB->BestEdge( -edge.normal ); 

				Edge referenceEdge, incidentEdge;
				bool flip = false;
				if( abs( DotProduct2D( e1.GetForwardVector(), edge.normal ) ) <= abs( DotProduct2D( e2.GetForwardVector(), edge.normal ) ) ) {
					referenceEdge = e1;
					incidentEdge = e2;
				}
				else {
					referenceEdge = e2;
					incidentEdge = e1;
					// we need to set a flag indicating that the reference
					// and incident edge were flipped so that when we do the final
					// clip operation, we use the right edge normal
					flip = true;
				}


				// the edge vector
				Vec2 refv = referenceEdge.GetForwardVector();
				refv.Normalize();

				float o1 = DotProduct2D( refv, referenceEdge.m_v1 );
				// clip the incident edge by the first
				// vertex of the reference edge
				std::vector<Vec2> clippedPoints;
				clippedPoints = clip( incidentEdge.m_v1, incidentEdge.m_v2, refv, o1 );

				// if we don't have 2 points left then fail
				if( clippedPoints.size() < 2 ) {
					ERROR_AND_DIE( "clippedPoints length is less than 2");
				};

				// clip whats left of the incident edge by the
				// second vertex of the reference edge
				// but we need to clip in the opposite direction
				// so we flip the direction and offset

				float o2 = DotProduct2D( refv, referenceEdge.m_v2 );
				clippedPoints = clip( clippedPoints[0], clippedPoints[1], -refv, -o2 );
				// if we dont have 2 points left then fail
				if( clippedPoints.size() < 2 ) {
					ERROR_AND_DIE( "clippedPoints length is less than 2" );
				};

				// get the reference edge normal
				Vec2 refNorm = refv.Cross( -1.f );

				// if we had to flip the incident and reference edges
				// then we need to flip the reference edge normal to
				// clip properly
				if( flip ) refNorm = -refNorm;
				// get the largest depth

				Vec2 refMax = Vec2::ZERO;
				//if( DotProduct2D( referenceEdge.m_v1, refNorm ) > DotProduct2D( referenceEdge.GetEndPoint(), refNorm ) ) // fix
				//{
				//	refMax = referenceEdge.m_v1;
				//}
				//else {
				//	refMax = referenceEdge.m_v2;
				//}

				float max = DotProduct2D( refNorm, referenceEdge.m_maxVertex );

				float depth0 = DotProduct2D( refNorm, clippedPoints[0] ) - max;
				float depth1 = DotProduct2D( refNorm, clippedPoints[1] ) - max;

				// make sure the final points are not past this maximum
				if( flip ) {
					if( depth1 < 0.f ) {
						clippedPoints.erase( clippedPoints.begin() + 1 );
					}
					if( depth0 < 0.f ) {
						clippedPoints.erase( clippedPoints.begin() );
					}
				}
				else {
					if( depth1 > 0.f ) {
						clippedPoints.erase( clippedPoints.begin() + 1 );
					}
					if( depth0 > 0.f ) {
						clippedPoints.erase( clippedPoints.begin() );
					}
				}


				//if( depth0 < 0.f ) {
				//	clippedPoints.erase( clippedPoints.begin() );
				//}
				//if( depth1 < 0.f ) {
				//	clippedPoints.erase( clippedPoints.begin() + 1 );
				//}
				
				mf.m_contactPoints = clippedPoints;
				return mf;
			}
			else {
				// we haven't reached the edge of the Minkowski Difference
				// so continue expanding by adding the new point to the simplex
				// in between the points that made the closest edge
				simplex.insert( simplex.begin() + edge.index, p );
			}
		}
	}

	// temp 
	return mf;
}

bool Collider2D::DoesAABB2sOverlap( AABB2 a, AABB2 b )
{
	return ( ( a.mins.x <= b.maxs.x && a.maxs.x >= b.mins.x ) &&
		( a.mins.y <= b.maxs.y && a.maxs.y >= b.mins.y ) );
}

Collider2D::~Collider2D()
{

}



static collision_check_cb gCollisionChecks[NUM_COLLIDER2D_TYPES * NUM_COLLIDER2D_TYPES] ={
	/*					disc,						polygon, */
	/*    disc */  DiscVsDiscCollisionCheck,      PolygonVsDiscCollisionCheck,
	/* polygon */  DiscVsPolygonCollisionCheck,   PolygonVsPolygonCollisionCheck
};

static manifold_cb gCollisionManifolds[NUM_COLLIDER2D_TYPES * NUM_COLLIDER2D_TYPES] ={
	/*					disc,						polygon, */
	/*    disc */  DiscVsDiscCollisionManifold,      PolygonVsDiscCollisionManifold,
	/* polygon */  DiscVsPolygonCollisionManifold,   PolygonVsPolygonCollisionManifold
};


bool Collider2D::Intersects( Collider2D const* other ) const
{
	if( !DoesAABB2sOverlap( this->GetWorldBounds(), other->GetWorldBounds() ) ) {
		return false;
	}

	eCollider2DType myType = GetType();
	eCollider2DType otherType = other->GetType();

	if( myType <= otherType ) // (Disc vs. Disc) or ( Disc vs. Polygon )
	{
		int idx = otherType * NUM_COLLIDER2D_TYPES + myType;
		collision_check_cb check = gCollisionChecks[idx];
		return check( this, other );
	}
	else 
	{
		// flip the types when looking into the index.
		int idx = myType * NUM_COLLIDER2D_TYPES + otherType;
		collision_check_cb check = gCollisionChecks[idx];
		return check( other, this );
	}
}

Manifold2 Collider2D::GetManifold( Collider2D const* other ) const
{
	if( !DoesAABB2sOverlap( this->GetWorldBounds(), other->GetWorldBounds() ) ) {
		return Manifold2();
	}

	eCollider2DType myType = GetType();
	eCollider2DType otherType = other->GetType();

	int idx = otherType * NUM_COLLIDER2D_TYPES + myType;
	manifold_cb manifold = gCollisionManifolds[idx];
	return manifold( this, other );

	//if( myType <= otherType ) // (Disc vs. Disc) or ( Disc vs. Polygon )
	//{
	//	int idx = otherType * NUM_COLLIDER2D_TYPES + myType;
	//	manifold_cb manifold = gCollisionManifolds[idx];
	//	return manifold( this, other );
	//}
	//else
	//{
	//	// flip the types when looking into the index.
	//	int idx = myType * NUM_COLLIDER2D_TYPES + otherType;
	//	manifold_cb manifold = gCollisionManifolds[idx];
	//	return manifold( other, this );
	//}
}

void Collider2D::SetMass( float newMass )
{
	m_mass =  newMass;
	m_rigidbody->m_moment = Calculatemoment( m_mass );
}

void Collider2D::SetColliderID( int id )
{
	m_colliderID = id;
}

void Collider2D::SetColliderIsTrigger( bool isTrigger )
{
	isTrigger ? (m_isTrigger = true) : (m_isTrigger = false);
}

void Collider2D::Move( Vec2 translation )
{
	m_rigidbody->m_worldPosition += translation;
}


float Collider2D::GetMass() const
{
	return m_mass;
}

float Collider2D::GetBounceWith( Collider2D const* other )
{
	return m_material.restitution * other->m_material.restitution;
}

float Collider2D::GetFrictionWith( Collider2D const* other )
{
	return m_material.friction * other->m_material.friction;
	//return sqrtf( (m_material.friction * m_material.friction) + (other->m_material.restitution * other->m_material.restitution) );
}
