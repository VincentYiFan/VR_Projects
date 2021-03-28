#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include <cimport.h>
#include <scene.h>
#include <postprocess.h>
#include <vector>


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
struct mesh_import_options_t
{
	Mat44 transform 			= Mat44::IDENTITY;
	bool invert_v				= false;
	//bool generate_normals		= false;
	bool generate_tangents		= false;
	bool invert_winding_order 	= false;
	bool clean					= false;  // optional
};
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
// load obj file 
void LoadOBJToVertexArray( std::vector<Vertex_PCUTBN>& out, std::vector<uint>& indices, char const* filename, mesh_import_options_t const& options );
void GenVerticesFromRawOBJ( std::vector<Vertex_PCUTBN>& oVerts, 
							const std::vector<Vec3>& iPositions,
							const std::vector<Vec2>& iTCoords,
							const std::vector<Vec3>& iNormals,
							std::string icurline );

// Triangulate a list of vertices into a face by printing
//	indices corresponding with triangles within it
void VertexTriangluation( std::vector<uint>& oIndices, const std::vector<Vertex_PCUTBN>& iVerts );
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------
// assimp
//-------------------------------------------------------------------------------------------------------------
//bool BInitAssimp();
void LoadModel( std::vector<Vertex_PCUTBN>& out, std::vector<uint32_t>& indices, char const* filePath, mesh_import_options_t const& options );
//-------------------------------------------------------------------------------------------------------------


void AppendAABBToVerts( std::vector<Vertex_PCU>& vertices, AABB2 aabb2 = AABB2::ZERO_TO_ONE ); // Default AABB2::ZERO_TO_ONE

void AppendIndexedCubeToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, AABB3 aabb3 );
void AppendIndexedCubeToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, AABB3 aabb3, Rgba8 color );
void AppendIndexedCubeWithNormalToVerts( std::vector<Vertex_PCUTBN>& vertices, std::vector<uint>& indices, AABB3 aabb3 );

void AppendOBB3ToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, OBB3 obb3 );
void AppendOBB3ToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, OBB3 obb3, Rgba8 color );

void AddUVSphereToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<uint>& indices, Vec3 center, float radius, uint horizintalCuts, uint verticalCuts, Rgba8 color );
void AddUVSphereToIndexedVertexArrayTBN( std::vector<Vertex_PCUTBN>& verts, std::vector<uint>& indices, Vec3 center, float radius, uint horizintalCuts, uint verticalCuts, Rgba8 color );

void AddIcosphereToVerts( std::vector<Vertex_PCU>& verts, std::vector<uint>& indices, Vec3 center, float radius, int subdivision, Rgba8 color );

void AppendQuadToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, Rgba8 color );
void AppendQuadToVerts( std::vector<Vertex_PCUTBN>& vertices, std::vector<uint>& indices, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, Rgba8 color );

void AppendLineToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 start, Vec3 end, Rgba8 start_color, Rgba8 end_color, float thickness  );
void AppendLineToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 start, Vec3 end, Rgba8 start_color, Rgba8 end_color, float startThickness, float endThickness );

void AppendConeToIndexedVertexArray( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 start, Vec3 end, Rgba8 start_color, Rgba8 end_color, float startThickness );

void AppendArrowToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 start, Vec3 end, Rgba8 start_color, Rgba8 end_color, float thickness );

void AddConeToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 center, float radius, Vec3 apexPoint, float height, Rgba8 startColor, Rgba8 endColor );

void AddVertices( std::vector<Vertex_PCU>& vertices, Vec3 v1, Vec3 v2, Vec3 v3 );
void AddVertices( std::vector<float> vertices, const float v1[3], const float v2[3], const float v3[3] );

void AddIndices( std::vector<uint>& indices, uint i1, uint i2, uint i3 );

void ComputeHalfVertex( const float v1[3], const float v2[3], float length, float newV[3] );

float ComputeScaleForLength( const float v[3], float length );

void CalculateTBN( Vec3& tangent, Vec3& bitangent, Vec3& normal, const Vec3& rawTangent, const Vec3& rawBitangent );