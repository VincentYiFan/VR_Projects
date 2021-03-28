#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/AABB2.hpp"

class ScreenDebugObject_Basis : public DebugRenderObject
{
public:
	ScreenDebugObject_Basis( Vec2 screen_origin_location, Mat44 basis_to_render, Rgba8 start_tint, Rgba8 end_tint, float duration );
	~ScreenDebugObject_Basis();

	virtual void DrawDebugObject() override;

public:
	Vec2 m_position = Vec2::ZERO;
	Vec2 m_iBasis = Vec2::ZERO;
	Vec2 m_jBasis = Vec2::ZERO;

	Rgba8 m_startTint = Rgba8::WHITE;
	Rgba8 m_endTint = Rgba8::WHITE;
};

