#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/Vec3.hpp"

class BasisDebugObject : public DebugRenderObject
{
public:
	BasisDebugObject( Mat44 basis, Rgba8 start_tint, Rgba8 end_tint, float thickness, float duration );
	~BasisDebugObject();

	virtual void DrawDebugObject() override;

public:
	Mat44 m_basis;

	Rgba8 m_startTint = Rgba8::WHITE;
	Rgba8 m_endTint = Rgba8::WHITE;

	float m_thickness = 0.1f;
};
