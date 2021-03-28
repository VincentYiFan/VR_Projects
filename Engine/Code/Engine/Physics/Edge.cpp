#include "Edge.hpp"

Edge::Edge( const Vec2& maxVertex, const Vec2& edgeVertex1, const Vec2& edgeVertex2 )
{
	m_maxVertex = maxVertex;
	m_v1 = edgeVertex1;
	m_v2 = edgeVertex2;
}

Vec2 Edge::GetForwardVector()
{
	return m_v2 - m_v1;
}

void Edge::operator=( const Edge& assignFrom )
{
	m_maxVertex=  assignFrom.m_maxVertex;
	m_v1 = assignFrom.m_v1;
	m_v2 = assignFrom.m_v2;
}
