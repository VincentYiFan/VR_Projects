#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"

struct Cone;

class ConeDebugObject : public DebugRenderObject
{
public:
	ConeDebugObject( const Cone& cone, Rgba8 startColor, Rgba8 endColor, float duration );
	~ConeDebugObject();

	virtual void DrawDebugObject() override;
};
