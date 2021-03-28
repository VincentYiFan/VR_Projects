#pragma once
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Transform.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"


class GameObject
{
public:
	~GameObject();

public:
	Rigidbody2D* m_rigidbody = nullptr;
	bool m_isCursorInside = false;
	bool m_isMovingByMouse = false;
	bool m_isCollidingWithOtherCollider = false;

	void SetMesh( GPUMesh* mesh );

	// 2D thing
	SpriteDefinition* definition; // equivalent of this, 3d object
	Vec2 m_position2D = Vec2::ZERO;
	float rotation = 0.f;

	// 3D equivalent
	GPUMesh* m_mesh;
	Vec3 m_position;
	Vec3 m_eulerRotation;

	// 3D equivalent using transform
	Transform m_transform;
};


