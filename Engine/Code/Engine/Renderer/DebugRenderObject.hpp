#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Transform.hpp"

enum eDebugRenderMode
{
	DEBUG_RENDER_ALWAYS,          // what is rendered always shows up
	DEBUG_RENDER_USE_DEPTH,       // respect the depth buffer
	DEBUG_RENDER_XRAY,            // renders twice - once darker when it should be hidden, and once more saturated when it should appear
};

class DebugRenderObject 
{
public:
	virtual ~DebugRenderObject();

public:
	virtual void DrawDebugObject() = 0;

	bool IsOld();
	bool IsGarbage() { return m_isGarbage; }
	void MarkForGarbage();

public:
	float m_size = 0.f;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	eDebugRenderMode m_mode = DEBUG_RENDER_USE_DEPTH;

	bool m_isGarbage = false;

	GPUMesh* m_mesh = nullptr;

	Timer* m_timer = nullptr;

	Transform m_transform;
};
