#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include <string>

class BitmapFont;

class BillboardTextDebugObject : public DebugRenderObject
{
public:
	BillboardTextDebugObject( Vec3 origin, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, char const* text );
	~BillboardTextDebugObject();

	virtual void DrawDebugObject() override;

public:
	Vec3 m_origin;
	Vec2 m_pivot =Vec2::ZERO;

	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;

	std::string m_text;

	BitmapFont* theFont = nullptr;
};
