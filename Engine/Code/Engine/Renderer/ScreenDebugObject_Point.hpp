#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/AABB2.hpp"

class ScreenDebugObject_Point : public DebugRenderObject
{
public:
	ScreenDebugObject_Point( Vec2 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration );
	~ScreenDebugObject_Point();

	virtual void DrawDebugObject() override;

public:
	Vec2 m_position = Vec2::ZERO;
	AABB2 m_box = AABB2::ZERO_TO_ONE;
};