#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/Vec3.hpp"

class SphereDebugObject : public DebugRenderObject
{
public:
	SphereDebugObject( Vec3 pos, float radius, Rgba8 startColor, Rgba8 endColor, float duration, bool isWireMode = false, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
	~SphereDebugObject();

	virtual void DrawDebugObject() override;

public:
	float m_radius = 0.f;
	bool m_isWireMode = false;
};