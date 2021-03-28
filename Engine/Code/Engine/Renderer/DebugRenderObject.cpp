#include "Engine/Renderer/DebugRenderObject.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/QuadDebugObject.hpp" 

DebugRenderObject::~DebugRenderObject()
{
	delete m_mesh;
	m_mesh = nullptr;

	delete m_timer;
	m_timer = nullptr;
}


bool DebugRenderObject::IsOld()
{
	return false;
}

void DebugRenderObject::MarkForGarbage()
{
	m_isGarbage = true;
}
