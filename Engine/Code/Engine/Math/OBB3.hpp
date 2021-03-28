#pragma once
#include "Engine/Math/Vec3.hpp"
#include <vector>

struct OBB3
{
public:
	Vec3 m_center;
	Vec3 m_iBasis = Vec3( 1.f, 0.f, 0.f );
	Vec3 m_jBasis = Vec3( 0.f, 1.f, 0.f );
	Vec3 m_kBasis = Vec3( 0.f, 0.f, 1.f );
	Vec3 m_halfDimensions;

	static const OBB3 ZERO_TO_ONE;

	OBB3() = default;
	~OBB3() = default;
	explicit OBB3( const Vec3& center, const Vec3& fullDimensions, const Vec3& iBasisNormal=Vec3( 1.f, 0.f, 0.f ), const Vec3& jBasisNormal=Vec3( 0.f, 1.f, 0.f ) );

	std::vector<Vec3> GetVertsPositions();
};
