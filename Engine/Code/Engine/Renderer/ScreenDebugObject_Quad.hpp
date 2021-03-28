#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/AABB2.hpp"

class Texture;

class ScreenDebugObject_Quad : public DebugRenderObject
{
public:
	ScreenDebugObject_Quad( AABB2 bounds, Rgba8 start_color, Rgba8 end_color, float duration );
	ScreenDebugObject_Quad( AABB2 bounds, Rgba8 tint, Texture* tex, AABB2 uvs, float duration );
	~ScreenDebugObject_Quad();

	virtual void DrawDebugObject() override;

public:
	Vec2 m_position = Vec2::ZERO;
	AABB2 m_box = AABB2::ZERO_TO_ONE;
	AABB2 m_uvs = AABB2::ZERO_TO_ONE;

	Rgba8 m_tint = Rgba8::WHITE;
	Texture* m_tex = nullptr;
};
