#pragma once
#include "Engine/Math/Vec3.hpp"


struct AABB3
{
public:
	Vec3 mins;
	Vec3 maxs;

public:
	~AABB3() = default;
	AABB3()	= default;
	AABB3( const AABB3& copyFrom );
	explicit AABB3( const Vec3& mins, const Vec3& maxs );


	void Translate( const Vec3& translation );
	Vec3 GetCenter() const;

	void	operator=( const AABB3& assignFrom );

	// Alternative way to represent an AABB
	// Vec3 center;  
	// Vec3 extents;
};