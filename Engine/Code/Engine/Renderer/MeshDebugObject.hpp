#pragma once
#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Math/Mat44.hpp"

class MeshDebugObject : public DebugRenderObject
{
public:
	MeshDebugObject( Mat44 model, GPUMesh* mesh, Rgba8 start_tint, Rgba8 end_tint, float duration );
	~MeshDebugObject();

	virtual void DrawDebugObject() override;

public:
	Mat44 m_model;
	Rgba8 m_startTint = Rgba8::WHITE;
	Rgba8 m_endTint = Rgba8::WHITE;
};
