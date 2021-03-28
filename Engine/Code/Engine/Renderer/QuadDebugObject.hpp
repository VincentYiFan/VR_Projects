#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"

class QuadDebugObject : public DebugRenderObject
{
public:
	QuadDebugObject( Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, AABB2 uvs, Rgba8 startColor, Rgba8 endColor, float duration );
	~QuadDebugObject();

	virtual void DrawDebugObject() override;

public:
	Vec3 m_p0 = Vec3::ZERO;
	Vec3 m_p1 = Vec3::ZERO;
	Vec3 m_p2 = Vec3::ZERO;
	Vec3 m_p3 = Vec3::ZERO;

	AABB2 m_uvs = AABB2::ZERO_TO_ONE;
};
