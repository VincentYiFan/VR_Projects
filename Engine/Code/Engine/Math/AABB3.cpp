#include "Engine/Math/AABB3.hpp"

AABB3::AABB3( const AABB3& copyFrom )
	:mins( copyFrom.mins )
	,maxs( copyFrom.maxs )
{
}

AABB3::AABB3( const Vec3& mins, const Vec3& maxs )
	: mins( mins )
	, maxs( maxs )
{
}

void AABB3::Translate( const Vec3& translation )
{
	mins += translation;
	maxs += translation;
}

Vec3 AABB3::GetCenter() const
{
	return (mins + maxs) * 0.5f;

}

void AABB3::operator=( const AABB3& assignFrom )
{
	mins = assignFrom.mins;
	maxs = assignFrom.maxs;
}
