#pragma once
#include "Engine/Math/Vec2.hpp"

struct LineSegment
{
public:
	Vec2 m_start;
	Vec2 m_forwardNormal;
	float m_length;

public:
	LineSegment() = default;
	explicit LineSegment( const Vec2& start, const Vec2& forwardNormal, float length );

	const Vec2 GetEndPoint() const;
	const Vec2 GetNearestPoint( const Vec2& referencePoint ) const;

	float GetMinX();
	float GetMaxX();
	float GetMinY();
	float GetMaxY();

	float CalculateYForX( float x );

	void Translate( const Vec2& translation );
	void operator=( const LineSegment& assignFrom );
};
