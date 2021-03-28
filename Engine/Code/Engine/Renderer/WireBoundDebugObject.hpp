#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"

struct AABB3;
struct OBB3;

class WireBoundDebugObject : public DebugRenderObject
{
public:
	WireBoundDebugObject( OBB3 obb3, Rgba8 startColor, Rgba8 endColor, float duration, bool isWireMode = true );
	WireBoundDebugObject( AABB3 aabb3, Rgba8 startColor, Rgba8 endColor, float duration, bool isWireMode = true );
	~WireBoundDebugObject();

	virtual void DrawDebugObject() override;

public:
	bool m_isWireMode = true;
};

