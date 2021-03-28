#pragma once
#include "Engine/Math/Vec3.hpp"

struct Cone
{
	Vec3	m_center = Vec3::ZERO;
	float	m_radius = 0.f;
	Vec3	m_apexPoint = Vec3::ZERO;
	float	m_height = 0.f;

	Cone() = default;
	~Cone() = default;
	explicit Cone( const Vec3& center, float radius, const Vec3& apexPoint, float height );

	Vec3 GetApexPoint() const;
};
