#pragma once
#include "Engine/Math/AABB2.hpp"
#include <vector>

struct OBB2 
{
public:
	Vec2 m_center;
	Vec2 m_iBasis = Vec2( 1.f, 0.f); // oriented i-basis normal; j-basis is +90 degrees ( -iBasis.y, iBasis.x )
	Vec2 m_halfDimensions; // half length of width and height. We store (halfWidth, halfHeight), so (width, height) = 2.f * m_halfDimensions

	static const OBB2 ZERO_TO_ONE;
	
	// Construction / destruction
	OBB2() = default;
	~OBB2() = default;
	OBB2( const OBB2& copyFrom );
	explicit OBB2( const Vec2& center, const Vec2& fullDimensions, const Vec2& iBasisNormal=Vec2( 1.f, 0.f ) );
	explicit OBB2( const Vec2& center, const Vec2& fullDimensions, float orientationDegrees = 0.f );
	explicit OBB2( const AABB2& asAxisAlignedBox, float orientationDegrees = 0.f );
	// Accessors (const methods)
	bool		IsPointInside( const Vec2& point ) const;
	const Vec2	GetCenter() const;
	const Vec2	GetDimensions() const;
	const Vec2	GetIBasisNormal() const;
	const Vec2	GetJBasisNormal() const;
	float		GetOrientationDegrees() const;
	const Vec2	GetNearestPoint( const Vec2& referencePos ) const;		// finds nearest point on/in box to reference position
	const Vec2	GetPointAtUV( const Vec2& uvCoordsZeroToOne ) const;	// pass (0,0) for relative mins, (.5,.5) for center, (1,1) for relative maxs
	const Vec2	GetUVForPoint( const Vec2& point ) const;				// returns (0,0) for relative mins, (.5,.5) for center, (1,1) for relative maxs
	float		GetOuterRadius() const;
	float		GetInnerRadius() const;
	void		GetCornerPositions( Vec2* out_fourPoints ) const;
	std::vector<Vec2> GetFourCornerPositions();

	// Mutators (non-const methods)
	void Translate( const Vec2& translation );
	void SetCenter( const Vec2& newCenter );
	void SetDimensions( const Vec2& newDimensions );
	void SetOrientationDegrees( float completelyNewAbsoluteOrientation );
	void RotateByDegrees( float relativeRotationDegrees );
	void StretchToIncludePoint( const Vec2& point );	// does not stretch if point is already on or in box
	void Fix(); // "corrects" the box: (1) abs() halfDimensions, and (2) normalize iBasis (or restore to 1,0 if currently 0,0)

	// Operators (self-mutating / non-const)
	void operator=( const OBB2& assignFrom );
};
