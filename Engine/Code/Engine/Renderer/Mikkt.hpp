#pragma once
#include "Engine/ThirdParty/mikktspace.h"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>

//----------------------------------------------------------------------------------------------------------------------------------
int		GetNumFaces( SMikkTSpaceContext const* pContext );

//----------------------------------------------------------------------------------------------------------------------------------
int		GetNumberOfVerticesForFace( SMikkTSpaceContext const* pContext, const int iFace );

//----------------------------------------------------------------------------------------------------------------------------------
void		GetPositionForFaceVert( const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert );

//----------------------------------------------------------------------------------------------------------------------------------
void		GetNormalForFaceVert( const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert );

//----------------------------------------------------------------------------------------------------------------------------------
void		GetUVForFaceVert( const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert );

//----------------------------------------------------------------------------------------------------------------------------------
void		SetTangent( const SMikkTSpaceContext* pContext,
							const float fvTangent[],
							const float fSign,
							const int	iFace, const int iVert );

//----------------------------------------------------------------------------------------------------------------------------------
void			GenerateTangentsForVertexArray( std::vector<Vertex_PCUTBN>& vertices );

