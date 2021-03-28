#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"

struct AABB2 
{
public:
	Vec2 mins;
	Vec2 maxs;

	static const AABB2 ZERO_TO_ONE;

public:
	~AABB2() = default;
	AABB2()	 = default;
	AABB2( const AABB2& copyFrom );  // copy constructor ( from another ivec2 )
	explicit AABB2 ( const Vec2& mins, const Vec2& maxs );  // explicit constructor (from minS, maxS ) 
	explicit AABB2 ( float minX, float minY, float maxX, float maxY );  //explicit constructor ( from x1,y1,x2,y2)

	bool		IsPointInside( const Vec2& point ) const;
	const Vec2	GetCenter() const;
	const Vec2  GetDimensions() const;
	const Vec2	GetNearestPoint( const Vec2& referencePosition ) const;
	const Vec2	GetPointAtUV( const Vec2& uvCoordsZeroToOne ) const;
	const Vec2	GetUVForPoint( const Vec2& point ) const;
	AABB2		GetBoxWithin( const Vec2& dimensions, const Vec2& alignment ) const;
	//AABB2		GetBoxAtLeft( float fractionOfWidth, float additionalWidth=0.f ) const;
	//AABB2		GetBoxAtRight(  float fractionOfWidth, float additionalWidth=0.f ) const;
	//AABB2		GetBoxAtBottom(  float fractionOfHeight, float additionalWidth=0.f ) const;
	//AABB2		GetBoxAtTop(  float fractionOfHeight, float additionalWidth=0.f ) const;

	// Mutators( non-const methods )
	void	Translate( const Vec2& translation );
	void	SetCenter( const Vec2& newCenter );
	void	SetDimensions( const Vec2& newDimesions );
	void	StretchToIncludePoint( const Vec2& point ); // does not stretch if the point is already on or in the box
	void	FitWithinBounds( const AABB2& bounds );
//	void	FitTightlyWithinBounds( const AABB2& box, const Vec2& alignment=ALIGN_CENTERED );
//	void	Resize( const Vec2& newDimensions, const Vec2& pivotAnchorAlignment=ALIGN_CENTERED );
	void	AlignWithinBox( const AABB2& box, const Vec2& alignent );
	AABB2	CraveBoxOffLeft( float fractionOfWidth, float additionalWidth=0.f );
	AABB2	CraveBoxOffRight( float fractionOfWidth, float additionalWidth=0.f );
	AABB2	CraveBoxOffBottom( float fractionOfHeight, float additionalHeight=0.f );
	AABB2	CraveBoxOffTop( float fractionOfHeight, float additionalHeight=0.f );


	bool	operator==( const AABB2& compareWith ) const;
	void	operator=( const AABB2& assignFrom );
}; 



