#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include <string>

class BitmapFont;

class TextDebugObject : public DebugRenderObject
{
public:
	TextDebugObject( Mat44 basis, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, char const* text );
	TextDebugObject( Mat44 basis, Vec2 pivot, Rgba8 color, float fontSize, char const* text, float duration = 0.f );
	~TextDebugObject();

	virtual void DrawDebugObject() override;

public:
	Mat44 m_basis;
	Vec2 m_pivot =Vec2::ZERO; 

	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;

	float m_fontSize = 0.5f;
	std::string m_text;
	BitmapFont* theFont = nullptr;
};
