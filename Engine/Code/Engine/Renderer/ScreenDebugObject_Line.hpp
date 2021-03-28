#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/AABB2.hpp"

class ScreenDebugObject_Line : public DebugRenderObject
{
public:
	ScreenDebugObject_Line( Vec2 p0, Rgba8 p0_color, Vec2 p1, Rgba8 p1_color, float duration );
	~ScreenDebugObject_Line();

	virtual void DrawDebugObject() override;

public:
	Vec2 m_position = Vec2::ZERO;
	Vec2 m_p0;
	Vec2 m_p1;

	Rgba8 m_p0_color = Rgba8::WHITE;
	Rgba8 m_p1_color = Rgba8::WHITE;
};
