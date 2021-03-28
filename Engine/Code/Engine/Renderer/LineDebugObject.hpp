#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/Vec3.hpp"

class LineDebugObject : public DebugRenderObject
{
public:
	LineDebugObject( Vec3 p0, Rgba8 p0_start_color, Vec3 p1, Rgba8 p1_start_color,float thickness, float duration, bool isWire = false );
	~LineDebugObject();

	virtual void DrawDebugObject() override;

public:
	Rgba8 m_p0_start_color = Rgba8::WHITE;
	Rgba8 m_p1_start_color = Rgba8::WHITE;

	bool m_isWire = false;
	float m_thickness = 0.f;
};
