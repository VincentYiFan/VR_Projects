#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

struct Vertex_PCUTBN
{
public:
	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;

	Vec3 m_tangent;
	Vec3 m_bitangent;
	Vec3 m_normal;

	static const buffer_attribute_t LAYOUT[];

	Vertex_PCUTBN() = default;  // default constructor added
	explicit Vertex_PCUTBN( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec3& tangent, const Vec3& bitangent, const Vec3& normal );
	explicit Vertex_PCUTBN( const Vec2& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec3& tangent, const Vec3& bitangent, const Vec3& normal );
};
