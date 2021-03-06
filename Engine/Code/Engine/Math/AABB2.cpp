#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"

const AABB2 AABB2::ZERO_TO_ONE( Vec2(0.f, 0.f), Vec2( 1.f, 1.f) );

AABB2::AABB2( const AABB2& copyFrom )
{
	mins = copyFrom.mins;
	maxs = copyFrom.maxs;
}

AABB2::AABB2( const Vec2& mins, const Vec2& maxs )
	:mins(mins)
	,maxs(maxs)
{
}

AABB2::AABB2( float minX, float minY, float maxX, float maxY )
	:mins(Vec2(minX,minY))
	,maxs(Vec2(maxX,maxY))
{	
}

bool AABB2::IsPointInside( const Vec2& point ) const
{
	return ( (point.x > mins.x && point.x < maxs.x) &&  (point.y > mins.y && point.y < maxs.y ) );
}

const Vec2 AABB2::GetCenter() const
{
	return Vec2(mins + maxs) * 0.5f;
}

const Vec2 AABB2::GetDimensions() const
{
	return Vec2( maxs.x - mins.x, maxs.y - mins.y ); 
}

const Vec2 AABB2::GetNearestPoint( const Vec2& referencePosition ) const
{
	float newX = Clamp( referencePosition.x, mins.x, maxs.x );
	float newY = Clamp( referencePosition.y, mins.y, maxs.y );
	return Vec2(newX, newY);
}

const Vec2 AABB2::GetPointAtUV( const Vec2& uvCoordsZeroToOne ) const
{
	return Vec2( uvCoordsZeroToOne.x * (maxs.x-mins.x) + mins.x
				,uvCoordsZeroToOne.y * (maxs.y-mins.y) + mins.y );
}

const Vec2 AABB2::GetUVForPoint( const Vec2& point ) const
{
	return Vec2( (point.x-mins.x) / (maxs.x-mins.x), (point.y-mins.y) / (maxs.y-mins.y));
}

AABB2 AABB2::GetBoxWithin( const Vec2& dimensions, const Vec2& alignment ) const
{
	AABB2 box = AABB2::ZERO_TO_ONE;
	Vec2 remainDimensions = GetDimensions() - dimensions;
	Vec2 translation = remainDimensions * alignment;
	box.mins = mins;
	box.maxs = box.mins + dimensions;
	box.Translate( translation );
	return box;
}


void AABB2::Translate( const Vec2& translation )
{
	mins += translation;
	maxs += translation;	
}

void AABB2::SetCenter( const Vec2& newCenter )
{
	Vec2 displacement = newCenter - GetCenter();
	mins += displacement;
	maxs += displacement;
}

void AABB2::SetDimensions( const Vec2& newDimesions )
{
	Vec2 halfNewDimesions = newDimesions * 0.5f;
	Vec2 centerPoint = GetCenter();
	mins.x = centerPoint.x - (halfNewDimesions.x);
	maxs.x = centerPoint.x + (halfNewDimesions.x);
	mins.y = centerPoint.y - (halfNewDimesions.y);
	maxs.y = centerPoint.y + (halfNewDimesions.y);
}

void AABB2::StretchToIncludePoint( const Vec2& point )
{
	if( IsPointInside(point) )
		return;
	if( point.x < mins.x )
	{
		mins.x = point.x;
	}
	else if( point.x > maxs.x ) 
	{
		maxs.x = point.x;
	}
	if( point.y < mins.y ) {
		mins.y = point.y;
	}
	else if( point.y > maxs.y ) {
		maxs.y = point.y;
	}

}

void AABB2::FitWithinBounds( const AABB2& bounds )
{
	if( mins.x < bounds.mins.x )
	{
		Translate( Vec2( bounds.mins.x - mins.x , 0.f ) );
	}
	if( maxs.x > bounds.maxs.x ) 
	{
		Translate( Vec2( bounds.maxs.x - maxs.x, 0.f ) );
	}
	if( mins.y < bounds.mins.y )
	{
		Translate( Vec2( 0.f, bounds.mins.y - mins.y ) );
	}
	if( maxs.y > bounds.maxs.y )
	{
		Translate( Vec2( 0.f, bounds.maxs.y - maxs.y) );
	}
}


void AABB2::AlignWithinBox( const AABB2& box, const Vec2& alignent )
{
	UNUSED(box);
	UNUSED(alignent);
}

AABB2 AABB2::CraveBoxOffLeft( float fractionOfWidth, float additionalWidth )
{
	UNUSED(fractionOfWidth);
	UNUSED(additionalWidth);
	return AABB2();
}

AABB2 AABB2::CraveBoxOffRight( float fractionOfWidth, float additionalWidth )
{
	float width = maxs.x - mins.x;
	float craveWidth = ( fractionOfWidth * width ) + additionalWidth;
	// Get version: return AABB2( maxs.x - craveWidth, mins.y, maxs.x, maxs.y ); 
	UNUSED ( width );
	UNUSED ( craveWidth );
	// #SD1ToDo
	return AABB2::ZERO_TO_ONE;
}

AABB2 AABB2::CraveBoxOffBottom( float fractionOfHeight, float additionalHeight )
{
	UNUSED(fractionOfHeight);
	UNUSED(additionalHeight);
	return AABB2();
}

AABB2 AABB2::CraveBoxOffTop( float fractionOfHeight, float additionalHeight )
{
	UNUSED( fractionOfHeight );
	UNUSED( additionalHeight );
	return AABB2();
}

bool AABB2::operator==( const AABB2& compareWith ) const
{
	if( compareWith.mins == mins && compareWith.maxs == maxs ) 
	{
		return true;
	}

	return false;
}

void AABB2::operator=( const AABB2& assignFrom )
{
	mins = assignFrom.mins;
	maxs = assignFrom.maxs;
}
