#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec4.hpp"
#include <string>

class ScreenDebugObject_Text : public DebugRenderObject
{
public:
	ScreenDebugObject_Text( Vec4 AlignmentOffest, Vec2 pivot, float textSize, Rgba8 start_color, Rgba8 end_color, float duration, char const* text );
	~ScreenDebugObject_Text();

	virtual void DrawDebugObject() override;

public:
	Vec4 m_alignmentOffset;
	Vec2 m_pivot = Vec2::ZERO;

	float m_textSize = 0.f;

	std::string m_text;
	BitmapFont* m_theFont = nullptr;

	
};