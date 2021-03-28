#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Mikkt.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
//#include <Importer.hpp>

void LoadOBJToVertexArray( std::vector<Vertex_PCUTBN>& out, std::vector<uint>& indices, char const* filename, mesh_import_options_t const& options )
{
	// check if the file extension is .obj
	char const* fileExtension = strstr( filename, ".obj" );
	if( !fileExtension ) {
		ERROR_AND_DIE( std::string( "Failed to open \"" + std::string(filename) + "\"" ).c_str() );
	}

	std::ifstream file( filename );
	if( !file.is_open() ) {
		ERROR_AND_DIE( "Error:Failed to load obj file" );
	}

	std::vector<Vec3> positions;
	std::vector<Vec2> tCoords;
	std::vector<Vec3> normals;

	//std::vector<Vertex_PCUTBN> vertices;
	//std::vector<uint> indices;

	std::string line;
	while( std::getline( file, line ) )
	{
		// Generate a Vertex Position
		if( firstToken( line ) == "v" ) 
		{
			std::vector<std::string> spos;
			Vec3 vpos;
			split( tail(line), spos, " " );

			vpos.x = std::stof( spos[0] );
			vpos.y = std::stof( spos[1] );
			vpos.z = std::stof( spos[2] );

			vpos = options.transform.TransformPosition3D( vpos );
			positions.push_back( vpos );
		}

		// Generate a Vertex Texture Coordinate
		if( firstToken( line ) == "vt" )
		{
			std::vector<std::string> stex;
			Vec2 vtex;
			split( tail( line ), stex, " " );

			vtex.x = std::stof( stex[0] );
			vtex.y = std::stof( stex[1] );

			if( options.invert_v ) {
				vtex.y = 1.f - vtex.y;
			}
			tCoords.push_back( vtex );
		}

		// Generate a Vertex Normal
		if( firstToken( line ) == "vn" )
		{
			std::vector<std::string> snor;
			Vec3 vnor;
			split( tail( line ), snor, " " );

			vnor.x = std::stof( snor[0] );
			vnor.y = std::stof( snor[1] );
			vnor.z = std::stof( snor[2] );

			normals.push_back( vnor );
		}

		// Generate a Face ( vertices & indices )
		if( firstToken( line ) == "f" )
		{
			// Generate the vertices
			std::vector<Vertex_PCUTBN> vVerts;
			GenVerticesFromRawOBJ( vVerts, positions, tCoords, normals, line );

			// Add Vertices
			for( int i = 0; i < int( vVerts.size() ); i++ )
			{
				// transform normal
				options.transform.TransformVector3D( vVerts[i].m_normal );
				out.push_back( vVerts[i] );
			}

			std::vector<uint> iIndices;

			VertexTriangluation(iIndices, vVerts);

			// Add Indices
			for( int i = 0; i < int( iIndices.size() ); i++ )
			{
				uint indnum = (uint)((out.size()) - vVerts.size()) + iIndices[i];
				indices.push_back( indnum );
			}
		}
	}

	
	// Generate Mikkt tangents
	if( options.generate_tangents ) {
		GenerateTangentsForVertexArray( out );
	}
	file.close();
}

void GenVerticesFromRawOBJ( std::vector<Vertex_PCUTBN>& oVerts, const std::vector<Vec3>& iPositions, const std::vector<Vec2>& iTCoords, const std::vector<Vec3>& iNormals, std::string icurline )
{
	std::vector<std::string> sface, svert;
	Vertex_PCUTBN vVert;
	split( tail( icurline ), sface, " " );

	// temp
	int numVertsOfFace = (int)sface.size();
	if( numVertsOfFace == 4 ) {
		std::string firstString = sface[0];
		std::string secondString = sface[2];
		sface.insert( sface.begin() + 2, firstString );
		sface.insert( sface.begin() + 2, secondString );
	}

	bool noNormal = false;

	// For every given vertex do this
	for( int i = 0; i < int( sface.size() ); i++ )
	{
		// See What type the vertex is.
		int vtype = 0;

		split( sface[i], svert, "/" );

		// Check for just position - v1
		if( svert.size() == 1 )
		{
			// Only position
			vtype = 1;
		}

		// Check for position & texture - v1/vt1
		if( svert.size() == 2 )
		{
			// Position & Texture
			vtype = 2;
		}

		// Check for Position, Texture and Normal - v1/vt1/vn1
		// or if Position and Normal - v1//vn1
		if( svert.size() == 3 )
		{
			if( svert[1] != "" )
			{
				// Position, Texture, and Normal
				vtype = 4;
			}
			else
			{
				// Position & Normal
				vtype = 3;
			}
		}
		  
		// Calculate and store the vertex
		switch( vtype )
		{
		case 1: // P
		{
			vVert.m_position = getElement( iPositions, svert[0] );
			vVert.m_uvTexCoords = Vec2( 0, 0 );
			noNormal = true;
			oVerts.push_back( vVert );
			break;
		}
		case 2: // P/T
		{
			vVert.m_position = getElement( iPositions, svert[0] );
			vVert.m_uvTexCoords = getElement( iTCoords, svert[1] );
			noNormal = true;
			oVerts.push_back( vVert );
			break;
		}
		case 3: // P//N
		{
			vVert.m_position = getElement( iPositions, svert[0] );
			vVert.m_uvTexCoords = Vec2( 0, 0 );
			vVert.m_normal = getElement( iNormals, svert[2] );
			oVerts.push_back( vVert );
			break;
		}
		case 4: // P/T/N
		{
			vVert.m_position = getElement( iPositions, svert[0] );
			vVert.m_uvTexCoords = getElement( iTCoords, svert[1] );
			vVert.m_normal = getElement( iNormals, svert[2] );
			oVerts.push_back( vVert );
			break;
		}
		default:
		{
			break;
		}
		}
	}

	// take care of missing normals
	// these may not be truly accurate but it is the 
	// best they get for not compiling a mesh with normals	
	if( noNormal )
	{
		Vec3 A = oVerts[0].m_position - oVerts[1].m_position;
		Vec3 B = oVerts[2].m_position - oVerts[1].m_position;

		Vec3 normal = CrossProduct( A, B );

		for( int i = 0; i < int( oVerts.size() ); i++ )
		{
			oVerts[i].m_normal = normal;
		}
	}	
}

void VertexTriangluation( std::vector<uint>& oIndices, const std::vector<Vertex_PCUTBN>& iVerts )
{
	// If there are 2 or less verts,
	// no triangle can be created,
	// so exit
	if( iVerts.size() < 3 )
	{
		return;
	}
	// If it is a triangle no need to calculate it
	if( iVerts.size() == 3 )
	{
		oIndices.push_back( 0 );
		oIndices.push_back( 1 );
		oIndices.push_back( 2 );
		return;
	}

	// Create a list of vertices
	std::vector<Vertex_PCUTBN> tVerts = iVerts;

	while( true )
	{
		// For every vertex
		for( int i = 0; i < int( tVerts.size() ); i++ )
		{
			// pPrev = the previous vertex in the list
			Vertex_PCUTBN pPrev;
			if( i == 0 )
			{
				pPrev = tVerts[tVerts.size() - 1];
			}
			else
			{
				pPrev = tVerts[i - 1];
			}

			// pCur = the current vertex;
			Vertex_PCUTBN pCur = tVerts[i];

			// pNext = the next vertex in the list
			Vertex_PCUTBN pNext;
			if( i == tVerts.size() - 1 )
			{
				pNext = tVerts[0];
			}
			else
			{
				pNext = tVerts[i + 1];
			}

			// Check to see if there are only 3 verts left
			// if so this is the last triangle
			if( tVerts.size() == 3 )
			{
				// Create a triangle from pCur, pPrev, pNext
				for( int j = 0; j < int( tVerts.size() ); j++ )
				{
					if( iVerts[j].m_position == pCur.m_position )
						oIndices.push_back( j );
					if( iVerts[j].m_position == pPrev.m_position )
						oIndices.push_back( j );
					if( iVerts[j].m_position == pNext.m_position )
						oIndices.push_back( j );
				}

				tVerts.clear();
				break;
			}
			if( tVerts.size() == 4 )
			{
				// Create a triangle from pCur, pPrev, pNext
				for( int j = 0; j < int( iVerts.size() ); j++ )
				{
					if( iVerts[j].m_position == pCur.m_position )
						oIndices.push_back( j );
					if( iVerts[j].m_position == pPrev.m_position )
						oIndices.push_back( j );
					if( iVerts[j].m_position == pNext.m_position )
						oIndices.push_back( j );
				}

				Vec3 tempVec;
				for( int j = 0; j < int( tVerts.size() ); j++ )
				{
					if( tVerts[j].m_position != pCur.m_position
						&& tVerts[j].m_position != pPrev.m_position
						&& tVerts[j].m_position != pNext.m_position )
					{
						tempVec = tVerts[j].m_position;
						break;
					}
				}

				// Create a triangle from pCur, pPrev, pNext
				for( int j = 0; j < int( iVerts.size() ); j++ )
				{
					if( iVerts[j].m_position == pPrev.m_position )
						oIndices.push_back( j );
					if( iVerts[j].m_position == pNext.m_position )
						oIndices.push_back( j );
					if( iVerts[j].m_position == tempVec )
						oIndices.push_back( j );
				}

				tVerts.clear();
				break;
			}

			// If Vertex is not an interior vertex
			float angle = GetAngleBetweenVec3( pPrev.m_position - pCur.m_position, pNext.m_position - pCur.m_position ) * ( 180 / PI );
			if( angle <= 0.f && angle >= 180.f )
				continue;

			// If any vertices are within this triangle
			bool inTri = false;
			for( int j = 0; j < int( iVerts.size() ); j++ )
			{
				if( IsPointInTriangle( iVerts[j].m_position, pPrev.m_position, pCur.m_position, pNext.m_position )
					&& iVerts[j].m_position != pPrev.m_position
					&& iVerts[j].m_position != pCur.m_position
					&& iVerts[j].m_position != pNext.m_position )
				{
					inTri = true;
					break;
				}
			}
			if( inTri )
				continue;

			// Create a triangle from pCur, pPrev, pNext
			for( int j = 0; j < int( iVerts.size() ); j++ )
			{
				if( iVerts[j].m_position == pCur.m_position )
					oIndices.push_back( j );
				if( iVerts[j].m_position == pPrev.m_position )
					oIndices.push_back( j );
				if( iVerts[j].m_position == pNext.m_position )
					oIndices.push_back( j );
			}

			// Delete pCur from the list
			for( int j = 0; j < int( tVerts.size() ); j++ )
			{
				if( tVerts[j].m_position == pCur.m_position )
				{
					tVerts.erase( tVerts.begin() + j );
					break;
				}
			}

			// reset i to the start
			// -1 since loop will add 1 to it
			i = -1;
		}

		// if no triangles were created
		if( oIndices.size() == 0 )
			break;

		// if no more vertices
		if( tVerts.size() == 0 )
			break;
	}
}

//bool BInitAssimp(  )
//{
//	const aiScene* scene = aiImportFile( "Data/Models/vr_controller_vive_1_5.obj", aiProcessPreset_TargetRealtime_MaxQuality );
//
//	if( !scene )
//	{
//		g_theConsole->Error( "Failed to load file, error: %s", aiGetErrorString() );
//		return false;
//	}
//	else
//	{
//		g_theConsole->Printf( "Number of meshes found in file: %d", scene->mNumMeshes );
//		g_theConsole->Printf( "Number of vertices in first mesh: %d", scene->mMeshes[0]->mNumVertices );
//	}
//	
//
//	return true;
//}

//-------------------------------------------------------------------------------------------------------------
void LoadModel( std::vector<Vertex_PCUTBN>& out, std::vector<uint32_t>& indices, char const* filePath, mesh_import_options_t const& options )
{
	UNUSED( options );

	const aiScene* scene = aiImportFile( filePath, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_CalcTangentSpace );
	if( !scene )
	{
		g_theConsole->Error( "Failed to load file, error: %s", aiGetErrorString() );
		return;
	}
	
	const aiMesh* mesh = scene->mMeshes[0];
	if( !mesh )
	{
		g_theConsole->Error( "Failed to find mesh, error: %s", aiGetErrorString() );
		return;
	}

	// Get vertices
	out.reserve( mesh->mNumVertices );
	for( uint32_t vertIdx = 0u; vertIdx < mesh->mNumVertices; ++vertIdx )
	{
		aiVector3D vert = mesh->mVertices[ vertIdx ];
		aiVector3D norm = mesh->mNormals[ vertIdx ];
		aiVector3D tang = mesh->mTangents[ vertIdx ];
		aiVector3D bitang = mesh->mBitangents[ vertIdx ];

		Vertex_PCUTBN vertex;
		vertex.m_position = Vec3( vert.x, vert.y, vert.z );
		vertex.m_normal = Vec3( norm.x, norm.y, norm.z );
		vertex.m_tangent = Vec3( tang.x, tang.y, tang.z );
		vertex.m_bitangent = Vec3( bitang.x, bitang.y, bitang.z );

		if( mesh->mTextureCoords[0] )
		{
			vertex.m_uvTexCoords.x = mesh->mTextureCoords[0][vertIdx].x;
			vertex.m_uvTexCoords.y = mesh->mTextureCoords[0][vertIdx].y;
		}

		out.push_back( vertex );
	}

	// Get indices
	indices.reserve( mesh->mNumFaces * 3u );
	for( uint32_t faceIdx = 0u; faceIdx < mesh->mNumFaces; ++faceIdx )
	{
		aiFace face = mesh->mFaces[faceIdx];
		assert( face.mNumIndices == 3u );

		for( uint32_t indiceIdx = 0u; indiceIdx < face.mNumIndices; indiceIdx++ )
		{
			indices.push_back( face.mIndices[indiceIdx] );
		}
	}

	//// Get Material
	//aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	//aiColor4D specularColor;
	//aiColor4D diffuseColor;
	//aiColor4D ambientColor;
	//float shininess;

	//aiGetMaterialColor( material, AI_MATKEY_COLOR_SPECULAR, &specularColor );
	//aiGetMaterialColor( material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor );
	//aiGetMaterialColor( material, AI_MATKEY_COLOR_AMBIENT, &ambientColor );
	//aiGetMaterialFloat( material, AI_MATKEY_SHININESS, &shininess );

	aiReleaseImport( scene );
	return;
}


void AppendAABBToVerts( std::vector<Vertex_PCU>& vertices, AABB2 aabb2 )
{
	vertices.push_back( Vertex_PCU( aabb2.mins, Rgba8::WHITE, Vec2::ZERO ) );
	vertices.push_back( Vertex_PCU( aabb2.maxs, Rgba8::WHITE, Vec2::ONE ) );

	vertices.push_back( Vertex_PCU( Vec2(aabb2.maxs.x, aabb2.mins.y), Rgba8::WHITE, Vec2( 1.f, 0.f ) ) ); // BottomRight
	vertices.push_back( Vertex_PCU( Vec2(aabb2.mins.x, aabb2.maxs.y), Rgba8::WHITE, Vec2( 0.f, 1.f ) ) ); // TopLeft
}

void AppendIndexedCubeToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, AABB3 aabb3 )
{
	Vec3& mins = aabb3.mins;
	Vec3& maxs = aabb3.maxs;

	// front
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2::ZERO ) );		// 0
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ) ) );	// 1
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ) ) );	// 2
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ) ) );	// 3


	// right
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ) );	// 7
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ) ) );	// 6
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ) ) );	// 5
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ) ) );	// 4

	
	
	// back
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ) ); // 9
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ) ) );	// 8
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ) ) );	// 11
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ) ) );	// 10


	// left
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, mins.z ), Rgba8::WHITE, Vec2::ZERO ) );		// 12
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ) ) ); // 13
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ) ) );	// 14
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ) ) );	// 15


	// top
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ) ); // 17
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ) ) );	// 16
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ) ) );	// 19
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ) ) );	// 18


	// bottom
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, mins.z ), Rgba8::WHITE, Vec2::ZERO ) );		// 20
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ) ) ); // 21
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ) ) );	// 22
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ) ) );	// 23


	uint cubeIndices[] =
	{
		0, 1, 2, 0, 2, 3,

		4, 5, 6, 4, 6, 7,

		8, 9, 10, 8, 10, 11,

		12, 13, 14, 12, 14, 15,

		16, 17, 18, 16, 18, 19,

		20, 21, 22, 20, 22, 23,
	};

	indices.insert( indices.begin(), std::begin( cubeIndices ), std::end( cubeIndices ) );
}

void AppendIndexedCubeToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, AABB3 aabb3, Rgba8 color )
{
	Vec3& mins = aabb3.mins;
	Vec3& maxs = aabb3.maxs;

	// front
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, maxs.z ), color, Vec2::ZERO ) );		
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, maxs.z ), color, Vec2( 1.f, 0.f ) ) );	
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, maxs.z ), color, Vec2( 1.f, 1.f ) ) );	
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, maxs.z ), color, Vec2( 0.f, 1.f ) ) );	

																								
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, maxs.z ), color, Vec2( 0.f, 0.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, mins.z ), color, Vec2( 1.f, 0.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, mins.z ), color, Vec2( 1.f, 1.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, maxs.z ), color, Vec2( 0.f, 1.f ) ) );


	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, mins.z ), color, Vec2( 0.f, 0.f ) ) ); 
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, mins.z ), color, Vec2( 1.f, 0.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, mins.z ), color, Vec2( 1.f, 1.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, mins.z ), color, Vec2( 0.f, 1.f ) ) );


	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, mins.z ), color, Vec2::ZERO ) );
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, maxs.z ), color, Vec2( 1.f, 0.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, maxs.z ), color, Vec2( 1.f, 1.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, mins.z ), color, Vec2( 0.f, 1.f ) ) );



	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, maxs.z ), color, Vec2( 0.f, 0.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, maxs.z ), color, Vec2( 1.f, 0.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, maxs.y, mins.z ), color, Vec2( 1.f, 1.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( mins.x, maxs.y, mins.z ), color, Vec2( 0.f, 1.f ) ) );


	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, mins.z ), color, Vec2::ZERO ) );
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, mins.z ), color, Vec2( 1.f, 0.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( maxs.x, mins.y, maxs.z ), color, Vec2( 1.f, 1.f ) ) );
	vertices.push_back( Vertex_PCU( Vec3( mins.x, mins.y, maxs.z ), color, Vec2( 0.f, 1.f ) ) );


	uint cubeIndices[] =
	{
		0, 1, 2, 0, 2, 3,

		4, 5, 6, 4, 6, 7,

		8, 9, 10, 8, 10, 11,

		12, 13, 14, 12, 14, 15,

		16, 17, 18, 16, 18, 19,

		20, 21, 22, 20, 22, 23,
	};

	indices.insert( indices.begin(), std::begin( cubeIndices ), std::end( cubeIndices ) );
}


void AppendIndexedCubeWithNormalToVerts( std::vector<Vertex_PCUTBN>& vertices, std::vector<uint>& indices, AABB3 aabb3 )
{
	Vec3& mins = aabb3.mins;
	Vec3& maxs = aabb3.maxs;

	Vec3 tangent;
	Vec3 bitangent;
	Vec3 normal;

	// 3 --- 2
	// |   / |
	// | /   |
	// 0 --- 1
	// front
	CalculateTBN( tangent, bitangent, normal, Vec3( maxs.x, mins.y, maxs.z ) - Vec3( mins.x, mins.y, maxs.z ), Vec3( mins.x, maxs.y, maxs.z ) - Vec3( mins.x, mins.y, maxs.z ) );
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ), tangent, bitangent, normal ) );	// 0
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ), tangent, bitangent, normal ) );	// 1
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ), tangent, bitangent, normal ) );	// 2
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ), tangent, bitangent, normal ) );	// 3

	// right
	CalculateTBN( tangent, bitangent, normal, Vec3( maxs.x, mins.y, mins.z ) - Vec3( maxs.x, mins.y, maxs.z ), Vec3( maxs.x, maxs.y, maxs.z ) - Vec3( maxs.x, mins.y, maxs.z ) );
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ), tangent, bitangent, normal ) );	// 4
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ), tangent, bitangent, normal ) );	// 5
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ), tangent, bitangent, normal ) );	// 6
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ), tangent, bitangent, normal ) );	// 7

	// back
	CalculateTBN( tangent, bitangent, normal, Vec3( mins.x, mins.y, mins.z ) - Vec3( maxs.x, mins.y, mins.z ), Vec3( maxs.x, maxs.y, mins.z ) -  Vec3( maxs.x, mins.y, mins.z ) );
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ), tangent, bitangent, normal ) );	// 8
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ), tangent, bitangent, normal ) );	// 9
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ), tangent, bitangent, normal ) );	// 10
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ), tangent, bitangent, normal ) );	// 11
	
	// left
	CalculateTBN( tangent, bitangent, normal, Vec3( mins.x, mins.y, maxs.z ) -  Vec3( mins.x, mins.y, mins.z ), Vec3( mins.x, maxs.y, mins.z ) -  Vec3( mins.x, mins.y, mins.z ) );
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ), tangent, bitangent, normal ) );	// 12
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ), tangent, bitangent, normal ) );	// 13
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ), tangent, bitangent, normal ) );	// 14
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ), tangent, bitangent, normal ) );	// 15
	
	// top
	CalculateTBN( tangent, bitangent, normal, Vec3( maxs.x, maxs.y, maxs.z ) - Vec3( mins.x, maxs.y, maxs.z ), Vec3( mins.x, maxs.y, mins.z ) - Vec3( mins.x, maxs.y, maxs.z ) );
	//vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ), tangent, bitangent, normal ) );	// 16
	//vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ), tangent, bitangent, normal ) );	// 17
	//vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ), tangent, bitangent, normal ) );	// 18
	//vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ), tangent, bitangent, normal ) );	// 19
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ), tangent, bitangent, normal ) );	// 16
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, maxs.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ), tangent, bitangent, normal ) );	// 17
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ), tangent, bitangent, normal ) );	// 18
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, maxs.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ), tangent, bitangent, normal ) );	// 19

	// bottom
	CalculateTBN( tangent, bitangent, normal,  Vec3( maxs.x, mins.y, mins.z ) - Vec3( mins.x, mins.y, mins.z ),  Vec3( mins.x, mins.y, maxs.z ) - Vec3( mins.x, mins.y, mins.z ) );
	//vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ), tangent, bitangent, normal ) );	// 20
	//vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ), tangent, bitangent, normal ) );	// 21
	//vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ), tangent, bitangent, normal ) );	// 22
	//vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ), tangent, bitangent, normal ) );	// 23
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 0.f, 0.f ), tangent, bitangent, normal ) );	// 20
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, mins.y, mins.z ), Rgba8::WHITE, Vec2( 1.f, 0.f ), tangent, bitangent, normal ) );	// 21
	vertices.push_back( Vertex_PCUTBN( Vec3( maxs.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 1.f, 1.f ), tangent, bitangent, normal ) );	// 22
	vertices.push_back( Vertex_PCUTBN( Vec3( mins.x, mins.y, maxs.z ), Rgba8::WHITE, Vec2( 0.f, 1.f ), tangent, bitangent, normal ) );	// 23


	uint cubeIndices[] =
	{
		0, 1, 2, 0, 2, 3,

		4, 5, 6, 4, 6, 7,

		8, 9, 10, 8, 10, 11,

		12, 13, 14, 12, 14, 15,

		16, 17, 18, 16, 18, 19,

		20, 21, 22, 20, 22, 23,
	};

	indices.insert( indices.begin(), std::begin( cubeIndices ), std::end( cubeIndices ) );
}

void AppendOBB3ToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, OBB3 obb3 )
{
	std::vector<Vec3> theVerts = obb3.GetVertsPositions();

	for( int i = 0; i < (int)theVerts.size(); ++i )
	{
		vertices.push_back( Vertex_PCU( theVerts[i], Rgba8::WHITE, Vec2::ZERO ) );
	}

	uint cubeIndices[] =
	{
		0, 1, 2, // -y
		0, 2, 3,

		1, 5, 6, // +y
		1, 6, 2,

		5, 4, 7, // -x
		5, 7, 6,

		4, 0, 3, // +x
		4, 3, 7,

		3, 2, 6, // -z
		3, 6, 7,

		1, 0, 4, // +z
		1, 4, 5,
	};

	indices.insert( indices.begin(), std::begin( cubeIndices ), std::end( cubeIndices ) );
}

void AppendOBB3ToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, OBB3 obb3, Rgba8 color )
{
	std::vector<Vec3> theVerts = obb3.GetVertsPositions();

	for( int i = 0; i < (int)theVerts.size(); ++i )
	{
		vertices.push_back( Vertex_PCU( theVerts[i], color, Vec2::ZERO ) );
	}

	uint cubeIndices[] =
	{
		0, 1, 2, // -y
		0, 2, 3,

		1, 5, 6, // +y
		1, 6, 2,

		5, 4, 7, // -x
		5, 7, 6,

		4, 0, 3, // +x
		4, 3, 7,

		3, 2, 6, // -z
		3, 6, 7,

		1, 0, 4, // +z
		1, 4, 5,
	};

	indices.insert( indices.begin(), std::begin( cubeIndices ), std::end( cubeIndices ) );
}

void AddUVSphereToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<uint>& indices, Vec3 center, float radius, uint horizintalCuts, uint verticalCuts, Rgba8 color )
{
	const float horizontalStep = 360.f / (float)horizintalCuts;
	const float verticalStep = 180.f / (float)verticalCuts;
	
	float x, y, z, xy = 0.f;
	float u, v = 0.f;

	float horizonralAngle = 0.f;
	float verticalAngle = 0.f;
	
	for( int i = 0; i <= (int)verticalCuts; ++i ) {
		verticalAngle = 90.f - i * verticalStep; // starting from 90.f to -90.f
		xy = radius * CosDegrees( verticalAngle );
		/*z*/y = radius * SinDegrees( verticalAngle );

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for( int j = 0; j <= (int)horizintalCuts; ++j ) {
			horizonralAngle = j * horizontalStep; // Starting from 0.f to 360.f

			x = xy * CosDegrees( horizonralAngle );
			/*y*/z = -xy * SinDegrees( horizonralAngle );

			// vertex tex coord (u, v) range between [0, 1]
			u = (float)j / (float)horizintalCuts;
			v = (float)i / (float)verticalCuts;

			verts.push_back( Vertex_PCU( Vec3( x, y, z ), color, Vec2( u , 1-v ) ) ); // 1-u, 1-v to flip

			// Vec3 normal =  Vec3(x,y,z).GetNormalized();
			// Vec3 tangent = Vec3( SinDegrees( horizonralAngle ), 0.f, -CosDegrees( horizonralAngle ) );
			// Vec3 bitangent = CrossProduct( N, T );
		}
	}

	// generate CCW index list of sphere triangles
	int k1, k2 = 0;
	for( int i = 0; i < (int)verticalCuts; ++i ) {
		k1 = i * ( horizintalCuts + 1 ); // beginning of current stack
		k2 = k1 + horizintalCuts + 1;	 // beginning of next stack

		for( int j = 0; j < (int)horizintalCuts; ++j, ++k1, ++k2 )
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if( i != 0 )
			{
				indices.push_back( k1 );
				indices.push_back( k2 );
				indices.push_back( k1 + 1 );
			}

			// k1+1 => k2 => k2+1
			if( i != (int)(verticalCuts -1) ) 
			{
				indices.push_back( k1 + 1 );
				indices.push_back( k2 );
				indices.push_back( k2 + 1 );
			}
		}
	}
}

void AddUVSphereToIndexedVertexArrayTBN( std::vector<Vertex_PCUTBN>& verts, std::vector<uint>& indices, Vec3 center, float radius, uint horizintalCuts, uint verticalCuts, Rgba8 color )
{
	const float horizontalStep = 360.f / (float)horizintalCuts;
	const float verticalStep = 180.f / (float)verticalCuts;

	float x, y, z, xy = 0.f;
	float u, v = 0.f;

	float horizonralAngle = 0.f;
	float verticalAngle = 0.f;

	for( int i = 0; i <= (int)verticalCuts; ++i ) {
		verticalAngle = 90.f - i * verticalStep; // starting from 90.f to -90.f
		xy = radius * CosDegrees( verticalAngle );
		/*z*/y = radius * SinDegrees( verticalAngle );

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for( int j = 0; j <= (int)horizintalCuts; ++j ) {
			horizonralAngle = j * horizontalStep; // Starting from 0.f to 360.f

			x = xy * CosDegrees( horizonralAngle );
			/*y*/z = -xy * SinDegrees( horizonralAngle );

			// vertex tex coord (u, v) range between [0, 1]
			u = (float)j / (float)horizintalCuts;
			v = (float)i / (float)verticalCuts;


			Vec3 normal =  Vec3( x, y, z ).GetNormalized();
			Vec3 tangent = Vec3( -SinDegrees( horizonralAngle ), 0.f, -CosDegrees( horizonralAngle ) );
			Vec3 bitangent = CrossProduct( normal, tangent );
			verts.push_back( Vertex_PCUTBN( Vec3( x, y, z ), color, Vec2( u, 1-v ), tangent, bitangent, normal ) ); // 1-u, 1-v to flip
		}
	}

	// generate CCW index list of sphere triangles
	int k1, k2 = 0;
	for( int i = 0; i < (int)verticalCuts; ++i ) {
		k1 = i * (horizintalCuts + 1); // beginning of current stack
		k2 = k1 + horizintalCuts + 1;	 // beginning of next stack

		for( int j = 0; j < (int)horizintalCuts; ++j, ++k1, ++k2 )
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if( i != 0 )
			{
				indices.push_back( k1 );
				indices.push_back( k2 );
				indices.push_back( k1 + 1 );
			}

			// k1+1 => k2 => k2+1
			if( i != (int)(verticalCuts -1) )
			{
				indices.push_back( k1 + 1 );
				indices.push_back( k2 );
				indices.push_back( k2 + 1 );
			}
		}
	}
}

void AddIcosphereToVerts( std::vector<Vertex_PCU>& verts, std::vector<uint>& indices, Vec3 center, float radius, int subdivision, Rgba8 color )
{
	UNUSED( indices );
	UNUSED( subdivision );

	const float H_ANGLE = 72.f;
	const float V_ANGLE = 26.565f; // atanf(1.0f / 2); elevation = 26.565 degree

	std::vector<float> vertices(12 * 3); // 12 vertices
	int i1, i2 = 0;				// indices
	float z, xy;				// coords

	float hAngle1 = -90.f - H_ANGLE * 0.5f;	// start from -126 deg at 2nd row
	float hAngle2 = -90.f;					// start from -90 deg at 3rd row

	/*Vec3 northPole = Vec3( 0.f, 0.f , radius );
	verts.push_back( Vertex_PCU( northPole, color, Vec2::ZERO) );*/

	// the first top vertex at (0, 0, r)
	vertices[0] = 0;
	vertices[1] = 0;
	vertices[2] = radius;

	// 10 vertices at 2nd and 3rd rows
	for( int i = 1; i <= 5; ++i )
	{
		i1 = i * 3;         // for 2nd row
		i2 = (i + 5) * 3;   // for 3rd row

		z = radius * SinDegrees( V_ANGLE );             // elevation
		xy = radius * CosDegrees( V_ANGLE );			// length on XY plane


		/*Vec3 point1 = Vec3( xy * cosf( hAngle1 ), xy * sinf( hAngle1 ), z );
		Vec3 point2 = Vec3( xy * cosf( hAngle2 ), xy * sinf( hAngle2 ), -z );
		verts.push_back( Vertex_PCU(point1, color, Vec2::ZERO ) );
		verts.push_back( Vertex_PCU(point2, color, Vec2::ZERO ) );*/

		vertices[i1] = xy * cosf( hAngle1 );      // x
		vertices[i2] = xy * cosf( hAngle2 );
		vertices[i1 + 1] = xy * sinf( hAngle1 );  // y
		vertices[i2 + 1] = xy * sinf( hAngle2 );
		vertices[i1 + 2] = z;					  // z
		vertices[i2 + 2] = -z;

		// next horizontal angles
		hAngle1 += H_ANGLE;
		hAngle2 += H_ANGLE;
	}

	// the last bottom vertex at (0, 0, -r)
	i1 = 11 * 3;
	vertices[i1] = 0;
	vertices[i1 + 1] = 0;
	vertices[i1 + 2] = -radius;

	/*Vec3 southPole = Vec3( 0.f, 0.f, -radius );
	verts.push_back( Vertex_PCU( southPole, color, Vec2::ZERO) );*/


	for( int idx = 0; idx < (int)vertices.size(); idx +=3 ) {
		Vec3 vertex = Vec3( vertices[idx], vertices[idx + 1], vertices[idx + 2] );
		verts.push_back( Vertex_PCU( vertex, color, Vec2::ZERO ) );
	}



	//// subdivision
	//std::vector<float> tmpVertices;
	//std::vector<uint> tmpIndices;

	//const float* v1, * v2, * v3;          // ptr to original vertices of a triangle
	//float newV1[3], newV2[3], newV3[3]; // new vertex positions
	//unsigned int index;

	//// iterate all subdivision levels
	//for( int i = 1; i <= subdivision; ++i )
	//{
	//	// copy prev vertex/index arrays and clear
	//	tmpVertices = vertices;
	//	tmpIndices = indices;
	//	vertices.clear();
	//	indices.clear();
	//	index = 0;

	//	// perform subdivision for each triangle
	//	for( int j = 0; j < tmpIndices.size(); j += 3 )
	//	{
	//		// get 3 vertices of a triangle
	//		v1 = &tmpVertices[tmpIndices[j] * 3];
	//		v2 = &tmpVertices[tmpIndices[j + 1] * 3];
	//		v3 = &tmpVertices[tmpIndices[j + 2] * 3];

	//		// compute 3 new vertices by spliting half on each edge
	//		//         v1       
	//		//        / \       
	//		// newV1 *---* newV3
	//		//      / \ / \     
	//		//    v2---*---v3   
	//		//       newV2      
	//		ComputeHalfVertex( v1, v2, radius, newV1 );
	//		ComputeHalfVertex( v2, v3, radius, newV2 );
	//		ComputeHalfVertex( v1, v3, radius, newV3 );

	//		// add 4 new triangles to vertex array
	//		AddVertices( vertices, v1, newV1, newV3 );
	//		AddVertices( vertices, newV1, v2, newV2 );
	//		AddVertices( vertices, newV1, newV2, newV3 );
	//		AddVertices( vertices, newV3, newV2, v3 );

	//		// add indices of 4 new triangles
	//		AddIndices( indices, index, index+1, index+2 );
	//		AddIndices( indices, index+3, index+4, index+5 );
	//		AddIndices( indices, index+6, index+7, index+8 );
	//		AddIndices( indices, index+9, index+10, index+11 );
	//		index += 12;    // next index
	//	}
	//}
}

void AppendQuadToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, Rgba8 color )
{
	vertices.push_back( Vertex_PCU( p0, color, Vec2( 0.f, 0.f ) ) );	// 0
	vertices.push_back( Vertex_PCU( p1, color, Vec2( 1.f, 0.f ) ) );	// 1
	vertices.push_back( Vertex_PCU( p2, color, Vec2( 1.f, 1.f ) ) );	// 2
	vertices.push_back( Vertex_PCU( p3, color, Vec2( 0.f, 1.f ) ) );	// 3
								    
	uint quadIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	indices.insert( indices.begin(), std::begin( quadIndices ), std::end( quadIndices ) );
}

void AppendQuadToVerts( std::vector<Vertex_PCUTBN>& vertices, std::vector<uint>& indices, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, Rgba8 color )
{
	//								   pos color       uv                  tangent                  bitangent               normal
	vertices.push_back( Vertex_PCUTBN( p0, color, Vec2( 0.f, 0.f ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 0
	vertices.push_back( Vertex_PCUTBN( p1, color, Vec2( 1.f, 0.f ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 1
	vertices.push_back( Vertex_PCUTBN( p2, color, Vec2( 1.f, 1.f ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 2
	vertices.push_back( Vertex_PCUTBN( p3, color, Vec2( 0.f, 1.f ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 3

	uint quadIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	indices.insert( indices.begin(), std::begin( quadIndices ), std::end( quadIndices ) );
}


void AppendLineToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 start, Vec3 end, Rgba8 start_color, Rgba8 end_color, float thickness )
{
	AppendLineToVerts( vertices, indices, start, end, start_color, end_color, thickness, thickness );
}

void AppendLineToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 start, Vec3 end, Rgba8 start_color, Rgba8 end_color, float startThickness, float endThickness )
{
	const float startRadius = startThickness * 0.5f;
	const float endRadius = endThickness * 0.5f;

	Mat44 lookAtMatrix = Mat44::CreateLookAtMatrix( start, end );
	Vec3 iBasis = lookAtMatrix.GetIBasis3D();
	Vec3 jBasis = lookAtMatrix.GetJBasis3D();

	// start disc
	constexpr float numSlices = 72.f;
	const float perSliceDegrees = 360.f / numSlices;
	//int startIndex = 0;
	int startIndex = (int)vertices.size();
	int endIndex = startIndex + (int)numSlices + 1;

	vertices.push_back( Vertex_PCU( start, start_color, Vec2::ZERO ) );

	for( float currentDegrees = 0.f; currentDegrees < 360.f; currentDegrees += perSliceDegrees )
	{
		Vec3 point = start + (iBasis * CosDegrees( currentDegrees ) + jBasis * SinDegrees( currentDegrees )) * startRadius;
		vertices.push_back( Vertex_PCU( point, start_color, Vec2::ZERO ) );
	}

	vertices.push_back( Vertex_PCU( end, end_color, Vec2::ZERO ) );

	for( float currentDegrees = 0.f; currentDegrees < 360.f; currentDegrees += perSliceDegrees )
	{
		Vec3 point = end + (iBasis * CosDegrees( currentDegrees ) + jBasis * SinDegrees( currentDegrees )) * endRadius;
		vertices.push_back( Vertex_PCU( point, end_color, Vec2::ZERO ) );
	}

	// indices for start disc
	for( int i = startIndex + 1; i < (int)numSlices; ++i )
	{
		indices.push_back( startIndex ); // start
		indices.push_back( i );
		indices.push_back( i + 1 );
	}

	indices.push_back( startIndex );
	indices.push_back( startIndex + (int)numSlices );
	indices.push_back( startIndex + 1 );


	// indices for end disc
	for( int i = endIndex + 1; i < (int)numSlices + endIndex; ++i )
	{
		indices.push_back( i - 1 );
		indices.push_back( i );
		indices.push_back( i + 1 );
	}

	indices.push_back( endIndex );
	indices.push_back( (int)numSlices + endIndex );
	indices.push_back( endIndex + 1 );


	// indices for the sides
	for( int i = endIndex + 1; i < endIndex + numSlices; ++i )
	{
		indices.push_back( i );
		indices.push_back( i + 1 );
		indices.push_back( i - (int)numSlices  );

		indices.push_back( i );
		indices.push_back( i - (int)numSlices );
		indices.push_back( i - (int)numSlices - 1 );
	}

	indices.push_back( endIndex + 1 );
	indices.push_back( endIndex + (int)numSlices );
	indices.push_back( endIndex - 1 );

	indices.push_back( endIndex + 1 );
	indices.push_back( endIndex - 1 );
	indices.push_back( endIndex - (int)numSlices );
}

void AppendConeToIndexedVertexArray( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 start, Vec3 end, Rgba8 start_color, Rgba8 end_color, float startThickness )
{
	AppendLineToVerts( vertices, indices, start, end, start_color, end_color, startThickness, 0.f );
}

void AppendArrowToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 start, Vec3 end, Rgba8 start_color, Rgba8 end_color, float thickness )
{
	Vec3 forward = end - start;
	Vec3 pointConeStart = start + (forward * 0.75f);

	Rgba8 halfColor;
	halfColor.r = unsigned char( (start_color.r + end_color.r) * 0.5 );
	halfColor.g = unsigned char( (start_color.g + end_color.g) * 0.5 );
	halfColor.b = unsigned char( (start_color.b + end_color.b) * 0.5 );
	halfColor.a = unsigned char( (start_color.a + end_color.a) * 0.5 );

	Rgba8 coneStartColor;
	coneStartColor.r = unsigned char( (halfColor.r + end_color.r) * 0.5 );
	coneStartColor.g = unsigned char( (halfColor.g + end_color.g) * 0.5 );
	coneStartColor.b = unsigned char( (halfColor.b + end_color.b) * 0.5 );
	coneStartColor.a = unsigned char( (halfColor.a + end_color.a) * 0.5 );

	// Line
	AppendLineToVerts( vertices, indices, start, pointConeStart, start_color, coneStartColor, thickness, thickness );

	// Cone
	AppendLineToVerts( vertices, indices, pointConeStart, end, coneStartColor, end_color, thickness * 2.f, 0.0f );
}

void AddConeToVerts( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, Vec3 center, float radius, Vec3 apexPoint, float height, Rgba8 startColor, Rgba8 endColor )
{
	UNUSED( height );
	UNUSED( indices );
	UNUSED( radius );

	// add the apex point to the array
	vertices.emplace_back( apexPoint, startColor, Vec2::ZERO ); 

	// #ToDo: UNIMPLEMENTED
	constexpr int n = 4;
	const float degreePerSlice = 360.f / (float)n;
	for( int i = 0; i < n; ++i )
	{
		// find those points and push back
	}
}

void AddVertices( std::vector<Vertex_PCU>& vertices, Vec3 v1, Vec3 v2, Vec3 v3 )
{
	vertices.push_back( Vertex_PCU( v1, Rgba8::WHITE, Vec2::ZERO ) );
	vertices.push_back( Vertex_PCU( v2, Rgba8::WHITE, Vec2::ZERO ) );
	vertices.push_back( Vertex_PCU( v3, Rgba8::WHITE, Vec2::ZERO ) );
}

void AddVertices( std::vector<float> vertices, const float v1[3], const float v2[3], const float v3[3] )
{
	vertices.push_back( v1[0] );  // x
	vertices.push_back( v1[1] );  // y
	vertices.push_back( v1[2] );  // z
	vertices.push_back( v2[0] );
	vertices.push_back( v2[1] );
	vertices.push_back( v2[2] );
	vertices.push_back( v3[0] );
	vertices.push_back( v3[1] );
	vertices.push_back( v3[2] );
}

void AddIndices( std::vector<uint>& indices, uint i1, uint i2, uint i3 )
{
	indices.push_back( i1 );
	indices.push_back( i2 );
	indices.push_back( i3 );
}

void ComputeHalfVertex( const float v1[3], const float v2[3], float length, float newV[3] )
{
	newV[0] = v1[0] + v2[0];
	newV[1] = v1[1] + v2[1];
	newV[2] = v1[2] + v2[2];
	float scale = ComputeScaleForLength( newV, length );
	newV[0] *= scale;
	newV[1] *= scale;
	newV[2] *= scale;
}

float ComputeScaleForLength( const float v[3], float length )
{
	// and normalize the vector then re-scale to new radius
	return length / sqrtf( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
}

void CalculateTBN( Vec3& tangent, Vec3& bitangent, Vec3& normal, const Vec3& rawTangent, const Vec3& rawBitangent )
{
	tangent = rawTangent.GetNormalized();
	bitangent = rawBitangent.GetNormalized();
	normal = CrossProduct( tangent, bitangent );
}
