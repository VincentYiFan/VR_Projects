#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/Vec3.hpp"

class ArrowDebugObject : public DebugRenderObject
{
public:
	ArrowDebugObject( Vec3 p0, Rgba8 p0_start_color, Vec3 p1, Rgba8 p1_start_color, float thickness, float duration );
	~ArrowDebugObject();

	virtual void DrawDebugObject() override;

public:
	Vec3 m_start = Vec3::ZERO;
	Vec3 m_end = Vec3::ZERO;

	Rgba8 m_p0_start_color = Rgba8::WHITE;

	Rgba8 m_p1_start_color = Rgba8::WHITE;

	float m_thickness = 0.f;
};
