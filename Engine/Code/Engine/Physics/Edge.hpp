#pragma once
#include "Engine/Math/Vec2.hpp"

struct Edge
{
public:
	Vec2 m_maxVertex;
	Vec2 m_v1;
	Vec2 m_v2;

public:
	Edge() = default;
	explicit Edge( const Vec2& maxVertex, const Vec2& edgeVertex1, const Vec2& edgeVertex2 );

	Vec2 GetForwardVector();

	void operator=( const Edge& assignFrom );
};

