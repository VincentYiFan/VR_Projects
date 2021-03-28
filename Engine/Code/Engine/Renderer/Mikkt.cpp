#include "Mikkt.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

int GetNumFaces( SMikkTSpaceContext const* pContext )
{
	//std::vector<Vertex_PCUTBN>* vertices = context->m_pUserData;
	std::vector<Vertex_PCUTBN>& vertices = *(std::vector<Vertex_PCUTBN>*)(pContext->m_pUserData);
	return (int)vertices.size() / 3;
}

int GetNumberOfVerticesForFace( SMikkTSpaceContext const* pContext, const int iFace )
{
	UNUSED(pContext);
	UNUSED(iFace);
	return 3;
}

void GetPositionForFaceVert( const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *(std::vector<Vertex_PCUTBN>*)(pContext->m_pUserData);
	int indexInVertexArray = iFace * 3 + iVert;

	Vec3 outPos = vertices[indexInVertexArray].m_position;

	fvPosOut[0] = outPos.x;
	fvPosOut[1] = outPos.y;
	fvPosOut[2] = outPos.z;
}

void GetNormalForFaceVert( const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *(std::vector<Vertex_PCUTBN>*)(pContext->m_pUserData);
	int indexInVertexArray = iFace * 3 + iVert;

	Vec3 outNorm = vertices[indexInVertexArray].m_normal;

	fvNormOut[0] = outNorm.x;
	fvNormOut[1] = outNorm.y;
	fvNormOut[2] = outNorm.z;
}

void GetUVForFaceVert( const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *(std::vector<Vertex_PCUTBN>*)(pContext->m_pUserData);
	int indexInVertexArray = iFace * 3 + iVert;

	Vec2 outUV = vertices[indexInVertexArray].m_uvTexCoords;

	fvTexcOut[0] = outUV.x;
	fvTexcOut[1] = outUV.y;
}

void SetTangent( const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *(std::vector<Vertex_PCUTBN>*)( pContext->m_pUserData);
	int indexInVertexArray = iFace * 3 + iVert;


	vertices[indexInVertexArray].m_tangent = Vec3( fvTangent[0], fvTangent[1], fvTangent[2] );
	vertices[indexInVertexArray].m_bitangent = CrossProduct( vertices[indexInVertexArray].m_normal, vertices[indexInVertexArray].m_tangent ) * fSign;
}

void GenerateTangentsForVertexArray( std::vector<Vertex_PCUTBN>& vertices )
{
	SMikkTSpaceInterface interface;
	interface.m_getNumFaces = GetNumFaces;
	interface.m_getNumVerticesOfFace = GetNumberOfVerticesForFace;
	interface.m_getPosition = GetPositionForFaceVert;
	interface.m_getNormal = GetNormalForFaceVert;
	interface.m_getTexCoord = GetUVForFaceVert;

	// MikkT telling us info
	interface.m_setTSpaceBasic = SetTangent;
	interface.m_setTSpace = nullptr;

	// Encapsulate ONE instance of running the algorithm
	SMikkTSpaceContext context;
	context.m_pInterface = &interface;
	context.m_pUserData = &vertices;

	// Run the algorithm
	genTangSpaceDefault( &context ); 
}
