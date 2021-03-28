#pragma once
#include "Engine/Math/Vec2.hpp"

struct Capsule2 
{
public:
	Vec2 m_boneStart;
	Vec2 m_boneEnd;
	float m_radius;

public:
	Capsule2() = default;
	explicit Capsule2( const Vec2& start, const Vec2& end, float radius );

	// Accessors
	bool IsPointInside( const Vec2& point ) const;
	const Vec2 GetNearestPoint( const Vec2& referencePoint ) const;

	// Mutators
	void Translate( const Vec2& translation );
	void RotateDegrees( float relativeRotationDegrees );

	void SetStartPosition( const Vec2& newStartPosition );
	void SetEndPosition( const Vec2& newEndPosition );
	void SetRadius( float newRadius );

	void operator=( const Capsule2& assignFrom );
};

