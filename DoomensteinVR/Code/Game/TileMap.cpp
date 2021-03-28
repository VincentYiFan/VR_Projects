#include "Game/TileMap.hpp"
#include "Game/Portal.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapMaterial.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/LineSegment.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include <cmath>
#include <limits>

#define RAYCAST_DISABLED

TileMap::TileMap( char const* mapName, XmlElement const& mapDef )
	:Map( mapName )
{
	CreateTiles( mapDef );
	PopulateTiles( mapDef );
	PopulateEntities( mapDef );

	m_worldMesh = new GPUMesh( g_theRenderer );
}

//-------------------------------------------------------------------------------------------------------------
TileMap::~TileMap()
{
	delete m_worldMesh;
	m_worldMesh = nullptr;
}

//-------------------------------------------------------------------------------------------------------------
void TileMap::Update( float deltaSeconds )
{
	ResolveEntityCollision();
	
	for( int i = 0; i < m_allEntities.size(); ++i )
	{
		Entity* entity = m_allEntities[i];
		if( entity != nullptr )
		{
			if( !entity->IsReadyToBeDeleted() )
			{
				entity->Update( deltaSeconds );
				PushEntityOutOfWalls( *entity );
			}
	

			if( entity->IsReadyToBeDeleted() && !entity->IsPlayer() )
			{
				RemoveEntityFromMap( entity );

				delete m_allEntities[i];
				m_allEntities[i] = nullptr;
			}
		}
	}


#ifndef RAYCAST_DISABLED
	//--------------------------------------------------------------------------------------------------------------------
	// Hack way to test Raycast
	Entity* player = m_allEntities[0];
	RaycastResult result = Raycast( player->m_position, player->GetForwardVector(), 2.f );
	Vec3 endPos;
	Vec3 startPos = Vec3( result.m_startPosition.x, result.m_startPosition.y, 0.3f );
	if( result.m_didImpact )
	{
		endPos = Vec3( result.m_impactPosition.x, result.m_impactPosition.y, 0.3f );
	}
	else
	{
		Vec2 endPos2D = player->m_position + 2.f * player->GetForwardVector();
		endPos = Vec3( endPos2D.x, endPos2D.y, 0.3f );
	}
	Rgba8 lineColor = ( result.m_didImpact ) ? Rgba8::RED : Rgba8::BLUE;
	//Vec3 endPos = Vec3( result.m_impactPosition.x, result.m_impactPosition.y, 0.3f );
	g_theDebugRenderSystem->DebugAddWorldLine( startPos, lineColor, endPos, lineColor, 0.1f, 0.f );
	//g_theConsole->PrintString( Rgba8::WHITE, Stringf( "%.2f, %.2f", result.m_impactPosition.x, result.m_impactPosition.y ) );

	//--------------------------------------------------------------------------------------------------------------------
	// Test Raycast entity
	Vec2 startPoint2D = player->m_position + player->m_radius * 1.f * player->GetForwardVector();
	Vec3 startPoint = Vec3( startPoint2D.x, startPoint2D.y, 0.3f ); 
	RaycastResult raycastResult = RaycastAgainstEntitiesXY( startPoint2D, player->GetForwardVector(), 2.f );
	//g_theDebugRenderSystem->DebugAddWorldPoint( startPoint, 0.01f,Rgba8::MAGENTA );
	if( raycastResult.m_didImpact )
	{
		Vec3 impactPoint = Vec3( raycastResult.m_impactPosition, 0.3f );
		g_theDebugRenderSystem->DebugAddWorldPoint( impactPoint, 0.01f,Rgba8::YELLOW );
	}

	//--------------------------------------------------------------------------------------------------------------------
	// Test Raycast Ceiling and Floor
	Vec3 cameraForward = g_theGame->m_worldCameraLeft.GetForwardVector();
	//cameraForward = Vec3( player->GetForwardVector(), 0.7f ).GetNormalized();
	Vec2 forwardVectorForCeilingAndFloor = Vec2( sqrtf( cameraForward.x * cameraForward.x + cameraForward.y * cameraForward.y ), cameraForward.z );
	RaycastResult ZTestResult = RaycastAgainstCeilingAndFloor( Vec2( 0.f, 0.65f ), forwardVectorForCeilingAndFloor, 10.f );
	if( ZTestResult.m_didImpact )
	{
		Vec3 impactPoint = Vec3( player->m_position, 0.65f ) + cameraForward * ZTestResult.m_impactFraction * ZTestResult.m_maxDistance;
		g_theDebugRenderSystem->DebugAddWorldPoint( impactPoint, 0.1f, Rgba8::GREEN );
	}
#endif
}

//-------------------------------------------------------------------------------------------------------------
void TileMap::UpdateMeshes()
{
	//int estimatedNumQuads = 4 * m_numTiles;
	//int estimatedNumTris = 2 * estimatedNumQuads;
	//int estimatedNumVerts = 3 * estimatedNumTris;
	//m_vertices.reserve( estimatedNumVerts );
	m_vertices.clear();

	for( int tileIndex = 0; tileIndex < m_numTiles; ++tileIndex )
	{
		AddVertsForTile( m_worldMesh, tileIndex );
	}

	if( !m_vertices.empty() )
	{
		m_worldMesh->UpdateVertices( m_vertices );
	}
	//if( !m_indices.empty() )
	//{
	//	m_worldMesh->UpdateIndices( m_indices );
	//}	
}

//-------------------------------------------------------------------------------------------------------------
void TileMap::Render( Camera& camera ) const
{
	g_theRenderer->SetModelMatrix( Mat44::IDENTITY );

	// Bind Diffuse and Normal Texture
	Texture* terrainTex = g_theRenderer->CreateOrGetTextureFromFile( "Data/Textures/Diffuse_4x4.png" );
	g_theRenderer->BindTexture( terrainTex );

	Texture* normalTex = g_theRenderer->CreateOrGetTextureFromFile( "Data/Textures/Normal_4x4.png" );
	g_theRenderer->BindNormalTexture( normalTex );

	if( m_worldMesh ) 
	{
		g_theRenderer->DrawMesh( m_worldMesh );
	}

	g_theRenderer->BindNormalTexture( g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/normal_flat.png") );
	for( int i = 0; i < (int) m_allEntities.size(); ++i )
	{
		if( m_allEntities[i] )
		{
			m_allEntities[i]->Render( camera );
			m_allEntities[i]->DebugRender( camera );
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
void TileMap::PushEntityOutOfWalls( Entity& e )
{
	if( !e.IsPushedByWalls() )
	{
		return;
	}

	IntVec2 const& pos = GetTileCoordsForWorldPosition( e.m_position );
	PushEntityOutOfTileIfSolid( e, pos + IntVec2( 1, 0 ) );
	PushEntityOutOfTileIfSolid( e, pos + IntVec2( 0, 1 ) );
	PushEntityOutOfTileIfSolid( e, pos + IntVec2( -1, 0 ) );
	PushEntityOutOfTileIfSolid( e, pos + IntVec2( 0, -1 ) );

	PushEntityOutOfTileIfSolid( e, pos + IntVec2( 1, 1 ) );
	PushEntityOutOfTileIfSolid( e, pos + IntVec2( -1, 1 ) );
	PushEntityOutOfTileIfSolid( e, pos + IntVec2( -1, -1 ) );
	PushEntityOutOfTileIfSolid( e, pos + IntVec2( 1, -1 ) );
}

//-------------------------------------------------------------------------------------------------------------
void TileMap::PushEntityOutOfTileIfSolid( Entity& e, IntVec2 const& tileCoords )
{
	if( IsTileSolid( tileCoords ) )
	{
		PushDiscOutOfAABB2D( e.m_position, e.m_radius, Get2DBoundsForTile( tileCoords ) );
	}
}

void TileMap::ParseLegend( std::map<char, MapRegionType const*>& legend, XmlElement const& mapDef )
{
	XmlElement const* legendElement = mapDef.FirstChildElement( "Legend" );
	for( XmlElement const* entry = legendElement->FirstChildElement(); entry; entry = entry->NextSiblingElement() )
	{
		// Error if not called Tile
		char glyph = ParseXmlAttribute( *entry, "glyph", '\0' );
		std::string regionTypeName = ParseXmlAttribute( *entry, "regionType", "" );
		MapRegionType const* regionType = MapRegionType::GetDefinitions( regionTypeName );
		if( regionType == nullptr )
		{
			g_theConsole->Error( "ERROR: Failed to find region type" );
			regionType = MapRegionType::GetDefaultRegionType();
		}

		legend[ glyph ] = regionType;
	}
}

//-------------------------------------------------------------------------------------------------------------
void TileMap::ParseMapRows( std::map< char, MapRegionType const* >& legend, XmlElement const& mapDef )
{
	XmlElement const* mapRows = mapDef.FirstChildElement( "MapRows" );
	if( mapRows == nullptr )
	{
		g_theConsole->Error( "Failed to find MapRows in map %s", m_mapName.c_str() );
	}
	int tileY = m_tileDimensions.y - 1;

	for( XmlElement const* mapRow = mapRows->FirstChildElement(); mapRow; mapRow = mapRow->NextSiblingElement() )
	{
		if( tileY < 0 )
		{
			g_theConsole->Error( "ERROR in map %s : map is %i high, but the row number doesn't match.", m_mapName.c_str(), m_tileDimensions.y );
			return;
		}
		 
		//if( mapRow->Name() != "MapRow" )
		if( std::strcmp( mapRow->Name(), "MapRow" ) != 0 )
		{
			g_theConsole->Error( "ERROR in map %s : child elements of <MapRows> must be <MapRow>, found <%s> instead", m_mapName.c_str(), mapRow->Name() );
			return;
		}

		std::string tiles = ParseXmlAttribute( *mapRow, "tiles", "" );
		if( tiles.length() != m_tileDimensions.x )
		{
			g_theConsole->Error( "ERROR in map %s : <MapRow tiles=\"%s\"> was %i wide, but required to be %i wide.", m_mapName.c_str(), tiles.c_str(), tiles.length(), m_tileDimensions.x );
			return;
		}

		for( int tileX = 0; tileX < m_tileDimensions.x; ++tileX )
		{
			int tileIndex = GetTileIndexForTileCoords( tileX, tileY );
			MapTile& tile = m_tiles[ tileIndex ];
			char glyph = tiles[ tileX ];
			auto found = legend.find( glyph );
			if( found == legend.end() )
			{
				//g_theConsole->Error( "ERROR in map %s : map tile '%c' in <MapRow tiles=\"%s\"> was not in <Legend>", m_mapName.c_str(), glyph, tiles.c_str() );
				//return;
				tile.m_type = MapRegionType::GetDefaultRegionType();
				continue;
			}

			tile.m_type = found->second;
		}

		m_mapRowsStr += tiles;
		--tileY;
	}

	if( tileY != -1 )
	{
		g_theConsole->Error( "ERROR in map %s : map is %i high, but had only %i <MapRow> elements.\n", m_mapName.c_str(), m_tileDimensions.y, m_tileDimensions.y - tileY );
		return;
	}
}

//-------------------------------------------------------------------------------------------------------------
//void TileMap::AddVertsForTile( Mesh_PCT& mesh, int tileIndex ) const
//{
//	MapTile const& tile = m_tiles[ tileIndex ];
//	if( tile.IsSolid() )
//	{
//		AddVertsForSolidTile( mesh, tile );
//	}
//	else
//	{
//		AddVertsForOpenTile( mesh, tile );
//	}
//}

//-------------------------------------------------------------------------------------------------------------
void TileMap::AddVertsForTile( GPUMesh* mesh, int tileIndex )
{
	MapTile const& tile = m_tiles[tileIndex];
	if( tile.IsSolid() )
	{
		AddVertsForSolidTile( mesh, tile );
	}
	else
	{
		AddVertsForOpenTile( mesh, tile );
	}
}

//-------------------------------------------------------------------------------------------------------------
//void TileMap::AddVertsForSolidTile( Mesh_PCT& mesh, MapTile const& tile ) const
//{
//	MapMaterial const* material = tile.m_type->GetSideMaterial();
//	AABB3 bounds = Get3DBoundsForTile( tile.m_tileCoords );
//	IntVec2 eastCoords = tile.m_tileCoords + IntVec2( 1, 0 );
//	IntVec2 westCoords = tile.m_tileCoords + IntVec2( -1, 0 );
//	IntVec2 northCoords = tile.m_tileCoords + IntVec2( 0, 1 );
//	IntVec2 southCoords = tile.m_tileCoords + IntVec2( 0, -1 );
//
//	Rgba8 tint = Rgba8::WHITE;
//	Vec2 minUV = material->GetUVAtMins();
//	Vec2 maxUV = material->GetUVAtMaxs();
//	float minU = minUV.x;
//	float minV = minUV.y;
//	float maxU = maxUV.x;
//	float maxV = maxUV.y;
//
//	for( float z = 0.f; z < bounds.maxs.z; z++ )
//	{
//		float minZ = z;
//		float maxZ = z + 1.f;
//		// Add east face
//		if( !IsTileSolid( eastCoords ) )
//		{
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( maxU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
//
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( minU, maxV ) );
//
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, minZ ), tint, Vec2( minU, minV ) );
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, minZ ), tint, Vec2( maxU, minV ) );
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, maxZ ), tint, Vec2( maxU, maxV ) );
//
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, minZ ), tint, Vec2( minU, minV ) );
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, maxZ ), tint, Vec2( maxU, maxV ) );
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, maxZ ), tint, Vec2( minU, maxV ) );
//		}
//		// Add west face
//		if( !IsTileSolid( westCoords ) )
//		{
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( maxU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
//
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( minU, maxV ) );
//
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, minZ ), tint, Vec2( minU, minV ) );
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, minZ ), tint, Vec2( maxU, minV ) );
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, maxZ ), tint, Vec2( maxU, maxV ) );
//
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, minZ ), tint, Vec2( minU, minV ) );
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, maxZ ), tint, Vec2( maxU, maxV ) );
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, maxZ ), tint, Vec2( minU, maxV ) );
//
//		}
//
//		// Add north face
//		if( !IsTileSolid( northCoords ) )
//		{
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( maxU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
//
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( minU, maxV ) );
//
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, minZ ), tint, Vec2( minU, minV ) );
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, minZ ), tint, Vec2( maxU, minV ) );
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, maxZ ), tint, Vec2( maxU, maxV ) );
//
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, minZ ), tint, Vec2( minU, minV ) );
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, maxZ ), tint, Vec2( maxU, maxV ) );
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, maxZ ), tint, Vec2( minU, maxV ) );
//
//		}
//
//		// Add south face
//		if( !IsTileSolid( southCoords ) )
//		{
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( maxU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
//			
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
//			//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
//			//mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( minU, maxV ) );
//
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, minZ ), tint, Vec2( minU, minV ) );
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, minZ ), tint, Vec2( maxU, minV ) );
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, maxZ ), tint, Vec2( maxU, maxV ) );
//
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, minZ ), tint, Vec2( minU, minV ) );
//			mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, maxZ ), tint, Vec2( maxU, maxV ) );
//			mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, maxZ ), tint, Vec2( minU, maxV ) );
//		}
//
//	}
//}

//-------------------------------------------------------------------------------------------------------------
void TileMap::AddVertsForSolidTile( GPUMesh* mesh, MapTile const& tile )
{
	UNUSED( mesh );

	MapMaterial const* material = tile.m_type->GetSideMaterial();
	AABB3 bounds = Get3DBoundsForTile( tile.m_tileCoords );
	IntVec2 eastCoords = tile.m_tileCoords + IntVec2( 1, 0 );
	IntVec2 westCoords = tile.m_tileCoords + IntVec2( -1, 0 );
	IntVec2 northCoords = tile.m_tileCoords + IntVec2( 0, 1 );
	IntVec2 southCoords = tile.m_tileCoords + IntVec2( 0, -1 );

	Rgba8 tint = Rgba8::WHITE;
	Vec2 minUV = material->GetUVAtMins();
	Vec2 maxUV = material->GetUVAtMaxs();
	float minU = minUV.x;
	float minV = minUV.y;
	float maxU = maxUV.x;
	float maxV = maxUV.y;

	Vec3 tangent;
	Vec3 bitangent;
	Vec3 normal;

	const Vec3& mins = bounds.mins;
	const Vec3& maxs = bounds.maxs;

	for( float z = 0.f; z < bounds.maxs.z; z += 1.f )
	{
		float minZ = z;
		float maxZ = z + 1.f;

		//mins.z = minZ;
		//maxs.z = maxZ;

		// Add east face
		if( !IsTileSolid( eastCoords ) )
		{
			CalculateTBN( tangent, bitangent, normal, Vec3( maxs.x, mins.y, minZ ) - Vec3( maxs.x, mins.y, maxZ ), Vec3( maxs.x, maxs.y, maxZ ) - Vec3( maxs.x, mins.y, maxZ ) );

			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, minZ ), tint, Vec2( minU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, minZ ), tint, Vec2( maxU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, maxZ ), tint, Vec2( maxU, maxV ), tangent, bitangent, normal );
																										 
			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, minZ ), tint, Vec2( minU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, maxZ ), tint, Vec2( maxU, maxV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, maxZ ), tint, Vec2( minU, maxV ), tangent, bitangent, normal );
		}
		// Add west face
		if( !IsTileSolid( westCoords ) )
		{
			CalculateTBN( tangent, bitangent, normal, Vec3( mins.x, mins.y, maxZ ) -  Vec3( mins.x, mins.y, minZ ), Vec3( mins.x, maxs.y, minZ ) -  Vec3( mins.x, mins.y, minZ ) );

			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, minZ ), tint, Vec2( minU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, minZ ), tint, Vec2( maxU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, maxZ ), tint, Vec2( maxU, maxV ), tangent, bitangent, normal );
																										
			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, minZ ), tint, Vec2( minU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, maxZ ), tint, Vec2( maxU, maxV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, maxZ ), tint, Vec2( minU, maxV ), tangent, bitangent, normal );
		
		}
		
		// Add north face
		if( !IsTileSolid( northCoords ) )
		{
			CalculateTBN( tangent, bitangent, normal, Vec3( mins.x, mins.y, minZ ) - Vec3( maxs.x, mins.y, minZ ), Vec3( maxs.x, maxs.y, minZ ) -  Vec3( maxs.x, mins.y, minZ ) );

			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, minZ ), tint, Vec2( minU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, minZ ), tint, Vec2( maxU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, maxZ ), tint, Vec2( maxU, maxV ), tangent, bitangent, normal );

			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, minZ ), tint, Vec2( minU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, maxZ ), tint, Vec2( maxU, maxV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, maxZ ), tint, Vec2( minU, maxV ), tangent, bitangent, normal );
		
		}
		
		// Add south face
		if( !IsTileSolid( southCoords ) )
		{
			CalculateTBN( tangent, bitangent, normal, Vec3( maxs.x, mins.y, maxZ ) - Vec3( mins.x, mins.y, maxZ ), Vec3( mins.x, maxs.y, maxZ ) - Vec3( mins.x, mins.y, maxZ ) );

			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, minZ ), tint, Vec2( minU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, minZ ), tint, Vec2( maxU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, maxZ ), tint, Vec2( maxU, maxV ), tangent, bitangent, normal );

			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, minZ ), tint, Vec2( minU, minV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, maxZ ), tint, Vec2( maxU, maxV ), tangent, bitangent, normal );
			m_vertices.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, maxZ ), tint, Vec2( minU, maxV ), tangent, bitangent, normal );
		}
	}
}


void TileMap::AddVertsForOpenTile( GPUMesh* mesh, MapTile const& tile )
{
	UNUSED( mesh );

	MapMaterial const* floorMaterial = tile.m_type->GetFloorMaterial();
	MapMaterial const* ceilingMaterial = tile.m_type->GetCeilingMaterial();
	AABB3 bounds = Get3DBoundsForTile( tile.m_tileCoords );

	Rgba8 tint = Rgba8::WHITE;

	Vec2 minUV = floorMaterial->GetUVAtMins();
	Vec2 maxUV = floorMaterial->GetUVAtMaxs();
	float minU = minUV.x;
	float minV = minUV.y;
	float maxU = maxUV.x;
	float maxV = maxUV.y;

	//								   pos													color       uv                  tangent                  bitangent               normal
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.mins.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( minU, minV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 0
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( maxU, minV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 1
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( maxU, maxV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 2

	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.mins.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( minU, minV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 0
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( maxU, maxV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 2
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( minU, maxV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 3

	/*uint quadIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};
	m_indices.insert( m_indices.begin(), std::begin( quadIndices ), std::end( quadIndices ) );*/

	// Add floor face
	//mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
	//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( maxU, minV ) );
	//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( maxU, maxV ) );
	//mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, bounds.mins.z ), tint, Vec2( minU, minV ) );
	//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( maxU, maxV ) );
	//mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), tint, Vec2( minU, maxV ) );

	// Change to ceiling props
	minUV = ceilingMaterial->GetUVAtMins();
	maxUV = ceilingMaterial->GetUVAtMaxs();
	minU = minUV.x;
	minV = minUV.y;
	maxU = maxUV.x;
	maxV = maxUV.y;

	//								   pos													color       uv                  tangent                  bitangent               normal
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( minU, minV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 0
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( maxU, minV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 1
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 2

	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( minU, minV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 0
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 2
	m_vertices.push_back( Vertex_PCUTBN( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( minU, maxV ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) ) );	// 3

	/*uint quadIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};
	m_indices.insert( m_indices.begin(), std::begin( quadIndices ), std::end( quadIndices ) );*/

	//// Add ceiling face
	//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( minU, minV ) );
	//mesh.emplace_back( Vec3( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( maxU, minV ) );
	//mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
	//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), tint, Vec2( minU, minV ) );
	//mesh.emplace_back( Vec3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( maxU, maxV ) );
	//mesh.emplace_back( Vec3( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), tint, Vec2( minU, maxV ) );
}

//-------------------------------------------------------------------------------------------------------------
bool TileMap::IsTileSolid( IntVec2 const& tileCoords ) const
{
	if( tileCoords.x < 0 || tileCoords.x >= m_tileDimensions.x || tileCoords.y < 0 || tileCoords.y >= m_tileDimensions.y )
	{
		return true; // assume out-of-bounds is always solid
	}

	int tileIndex = GetTileIndexForTileCoords( tileCoords.x, tileCoords.y );
	MapTile const& tile = m_tiles[ tileIndex ];
	return tile.IsSolid();
}

//-------------------------------------------------------------------------------------------------------------
int TileMap::GetTileIndexForTileCoords( int tileX, int tileY ) const
{
	return tileX + ( m_tileDimensions.x * tileY );
}

//-------------------------------------------------------------------------------------------------------------
AABB3 TileMap::Get3DBoundsForTile( IntVec2 tileCoords ) const
{
	Vec3 mins = Vec3( (float)tileCoords.x, (float)tileCoords.y, 0.f );
	Vec3 maxs = Vec3( (float)tileCoords.x + 1.f, (float)tileCoords.y + 1.f, CEILING_HEIGHT );
	//Vec3 maxs = Vec3( (float)tileCoords.x + 1.f, (float)tileCoords.y + 1.f, 1.f );
	AABB3 bounds = AABB3( mins, maxs );
	return bounds; 
}

//-------------------------------------------------------------------------------------------------------------
AABB2 TileMap::Get2DBoundsForTile( IntVec2 tileCoords ) const
{
	Vec2 mins = Vec2( (float)tileCoords.x, (float)tileCoords.y );
	Vec2 maxs = Vec2( (float)( tileCoords.x + 1 ), (float)( tileCoords.y + 1 ) );
	return AABB2( mins, maxs );
}

//-------------------------------------------------------------------------------------------------------------
IntVec2 TileMap::GetTileCoordsForWorldPosition( Vec2 const& worldPosition )
{
	return IntVec2( (int)worldPosition.x, (int)worldPosition.y );
}

//-------------------------------------------------------------------------------------------------------------
RaycastResult TileMap::Raycast( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance )
{
	RaycastResult raycastResult;
	raycastResult.m_startPosition = start;
	raycastResult.m_forwardNormal = forwardDirection.GetNormalized();
	raycastResult.m_maxDistance = maxDistance;

	// Compute Integer x,y tileCoords for start position x,y
	IntVec2 tileCoords = IntVec2( (int)std::floor( start.x ), (int)std::floor( start.y ) );

	// Check if starting tile is solid
	// if so, fill out your raycastResult and return
	if( IsTileSolid( tileCoords ) ) 
	{
		raycastResult.m_didImpact = true;
		raycastResult.m_impactPosition = start;
		raycastResult.m_impactFraction = 0.f;
		raycastResult.m_impactDistance = 0.f;
		raycastResult.m_impactSurfaceNormal = -raycastResult.m_forwardNormal;
		return raycastResult;
	}
	
	// Compute rayDisplacement ("D") as forwardDirection * maxDistance
	Vec2 rayDisplacment = raycastResult.m_forwardNormal * maxDistance;

	// Compare = float tPerXCrossing = 1 / abs(D.x)
	// this is "how much T it takes to cross each X line"
	float tPerXCrossing = 1 / fabsf( rayDisplacment.x );

	// Compute int tileStepX to be either +1 or -1 
	// depending on direction of ray (e.g. +1 if D.x is positive, -1 otherwise)
	// this is "one tile stop forward in the x direction"
	int tilsStepX = ( rayDisplacment.x > 0 ) ? 1 : -1;

	// Compute int offsetToLeadingEdgeX
	// (tileStepX + 1) / 2
	// 1 if to the right, 0 if to the left
	int offsetToLeadingEdgeX = (tilsStepX + 1) / 2;

	// Compute firstVerticalIntersectionX as:
	// (float)( startTileX + offsetToLeadingEdgeX )
	float firstVerticalIntersectionX =  (float)( tileCoords.x + offsetToLeadingEdgeX );

	// Compute tOfNextXCrossing
	float tOfNextXCrossing = fabsf( firstVerticalIntersectionX - start.x ) * tPerXCrossing;

	// Repeat steps for Y, same as for X
	float tPerYCrossing = 1 / fabsf( rayDisplacment.y );
	int tilsStepY = ( rayDisplacment.y > 0 ) ? 1 : -1;
	int offsetToLeadingEdgeY = (tilsStepY + 1) / 2;
	float firstHorizontallIntersectionY =  (float)( tileCoords.y + offsetToLeadingEdgeY );
	float tOfNextYCrossing = fabsf( firstHorizontallIntersectionY - start.y ) * tPerYCrossing;

	// Main Raycast Loop (after initialization)
	while( true )
	{
		if( tOfNextXCrossing < tOfNextYCrossing )
		{
			if( tOfNextXCrossing > 1.f )
			{
				// No Impact
				raycastResult.m_didImpact = false;
				raycastResult.m_impactDistance = maxDistance;
				return raycastResult;
			}
			else
			{
				tileCoords.x += tilsStepX;
				if( IsTileSolid( tileCoords ) )
				{
					// Impact
					raycastResult.m_didImpact = true;
					raycastResult.m_impactPosition = start + ( rayDisplacment * tOfNextXCrossing );
					raycastResult.m_impactTileCoords = tileCoords;
					raycastResult.m_impactFraction = tOfNextXCrossing;
					raycastResult.m_impactDistance = tOfNextXCrossing * maxDistance;
					raycastResult.m_impactSurfaceNormal = ( tilsStepX == 1 ) ? Vec2( -1.f, 0.f ) : Vec2( 1.f, 0.f );
					return raycastResult;
				}
				else
				{
					tOfNextXCrossing += tPerXCrossing;
				}
			}
		}
		else
		{
			if( tOfNextYCrossing > 1.f )
			{
				// No Impact
				raycastResult.m_didImpact = false;
				raycastResult.m_impactDistance = maxDistance;
				return raycastResult;
			}
			else
			{
				tileCoords.y += tilsStepY;
				if( IsTileSolid( tileCoords ) )
				{
					// Impact
					raycastResult.m_didImpact = true;
					raycastResult.m_impactPosition = start + ( rayDisplacment * tOfNextYCrossing );
					raycastResult.m_impactTileCoords = tileCoords;
					raycastResult.m_impactFraction = tOfNextYCrossing;
					raycastResult.m_impactDistance = tOfNextYCrossing * maxDistance;
					raycastResult.m_impactSurfaceNormal = (tilsStepY == 1) ? Vec2( 0.f, -1.f ) : Vec2( 0.f, 1.f );
					return raycastResult;
				}
				else
				{
					tOfNextYCrossing += tPerYCrossing;
				}
			}
		}
	}
}

RaycastResult TileMap::RaycastAgainstEntities( Vec3 const& start, Vec3 const& forwardDirection, float maxDistance )
{
	UNUSED( maxDistance );
	RaycastResult result;
	//RaycastResult resultXY;
	//RaycastResult resultZ;

	//Vec2 forwardDirection2D = Vec2( forwardDirection.x, forwardDirection.y ).GetNormalized();
	//Vec3 forwardDirectionXY = Vec3( forwardDirection2D.x, forwardDirection2D.y, 0.f );

	//float maxDistanceXY = GetProjectedLength3D( forwardDirection, forwardDirectionXY ) * maxDistance;

	//Vec2 startPos2D = Vec2( start.x , start.y );
	//Vec3 startPosXY = Vec3( start.x, start.y, 0.f );

	//g_theDebugRenderSystem->DebugAddWorldLine( startPosXY, Rgba8::RED, startPosXY + forwardDirectionXY * maxDistanceXY,  Rgba8::RED, 0.003f, 1.f );
	//resultXY = RaycastAgainstEntities2D_Updated( startPos2D, forwardDirection2D, maxDistanceXY );


	// ----- Ray vs. AABB3 ----- 
	Ray r = Ray( start, forwardDirection );
	float t = std::numeric_limits<float>::infinity();

	for( int i = 0; i < (int)m_allEntities.size(); ++i )
	{
		if( m_allEntities[i] == nullptr ) {
			continue;
		}
		if( m_allEntities[i]->m_isPlayer ) {
			continue;
		}
		if( m_allEntities[i]->m_isProjectile ) {
			continue;
		}

		Entity& e = *m_allEntities[i];
		Vec3 min = Vec3( e.m_position.x - e.m_radius, e.m_position.y - e.m_radius, 0.f );
		Vec3 max = Vec3( e.m_position.x + e.m_radius, e.m_position.y + e.m_radius, e.m_height );
		Box3 box = Box3( min, max );

		float tEntity; 

		if( DoesRayAndAABB2Intersect( r, box, tEntity ) )
		{
			if( tEntity <= t )
			{
				result.m_didImpact = true;
				result.m_impactFraction = tEntity;
				result.m_impactEntity = m_allEntities[i];
			}
		}
	}
	
	return result;	
}

RaycastResult TileMap::RaycastAgainstEntities2D( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance )
{
	//-------------------------------------------------------------------------------------------------------------
	//https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection-algorithm
	//-------------------------------------------------------------------------------------------------------------
	RaycastResult result;
	result.m_startPosition = start;
	result.m_forwardNormal = forwardDirection.GetNormalized();
	result.m_maxDistance = maxDistance;

	Vec2 endPos = start + result.m_forwardNormal * maxDistance;

	float t = 1.0f;

	// For loop through all the entities
	for( int i = 0; i < m_allEntities.size(); ++i )
	{
		if( m_allEntities[i] == nullptr ) {
			continue;
		}
		if( m_allEntities[i]->m_isPlayer ) {
			continue;
		}

		Vec2& d = result.m_forwardNormal;
		Vec2 f = start - m_allEntities[i]->m_position;
		float const& r = m_allEntities[i]->m_radius;

		float a = DotProduct2D( d, d );
		float b = 2 * DotProduct2D( f, d );
		float c = DotProduct2D( f, f ) - r * r;

		float discriminant = b * b - 4 * a * c;
		if( discriminant < 0 )
		{
			// no intersection
			continue;
		}
		else 
		{
			discriminant = sqrtf( discriminant );

			// either solution may be on or off the ray so need to test both
			// t1 is always the smaller value, because BOTH discriminant and
			// a are nonnegative.
			float t1 = (-b - discriminant)/(2*a);
			float t2 = (-b + discriminant)/(2*a);

			if( t1 >= 0 && t1 <= 1 )
			{
				// t1 is the intersection, and it's closer than t2
				t = t1;
			}
			
			// here t1 didn't intersect so we are either started
			// inside the sphere or completely past it
			if( t2 >= 0 && t2 <= 1 )
			{
				// ExitWound
			}
		}
	
	}

	if( t < 1.0f )
	{
		// Impact
		result.m_didImpact = true;
		result.m_impactPosition = start + t * result.m_forwardNormal;
	}

	return result;
}

RaycastResult TileMap::RaycastAgainstEntities2D_Updated( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance )
{
	RaycastResult result;
	result.m_startPosition = start;
	result.m_forwardNormal = forwardDirection.GetNormalized();
	result.m_maxDistance = maxDistance;

	Vec2 endPos = start + result.m_forwardNormal * maxDistance;

	float t = 1.0f;

	// For loop through all the entities
	for( int i = 0; i < m_allEntities.size(); ++i )
	{
		if( m_allEntities[i] == nullptr ) {
			continue;
		}
		if( m_allEntities[i]->m_isPlayer ) {
			continue;
		}

		Vec2 f = start - m_allEntities[i]->m_position;
		float const& r = m_allEntities[i]->m_radius;
		float h = m_allEntities[i]->m_position.x;
		float k = m_allEntities[i]->m_position.y;

		float a = ( endPos.x - start.x ) * ( endPos.x - start.x ) + ( endPos.y - start.y ) * ( endPos.y - start.y );
		float b = 2 * ( endPos.x - start.x ) * ( start.x - h ) + 2 * ( endPos.y - start.y ) * ( start.y - k );
		float c = ( start.x - h ) * ( start.x - h ) + ( start.y - k ) * ( start.y - k ) - ( r * r );

		float discriminant = b * b - 4 * a * c;
		if( discriminant < 0 )
		{
			// no intersection
			continue;
		}
		else
		{
			discriminant = sqrtf( discriminant );

			float t1 = 2 * c / ( -b + discriminant );

			if( t1 >= 0 && t1 <= 1 )
			{
				t = t1;

				result.m_didImpact = true;
				result.m_impactPosition.x = ( endPos.x - start.x ) * t + start.x;
				result.m_impactPosition.y = ( endPos.y - start.y ) * t + start.y;
			}
		}

	}

	return result;
}

//-------------------------------------------------------------------------------------------------------------
RaycastResult TileMap::RaycastAgainstEntitiesZ( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance )
{
	RaycastResult result;
	result.m_startPosition = start;
	result.m_forwardNormal = forwardDirection.GetNormalized();
	result.m_maxDistance = maxDistance;

	//float nearestDistSquared = 99999999.f; // A large number

	for( int i = 0; i < m_allEntities.size(); ++i )
	{
		Entity& e = *m_allEntities[i];
		float posXY = sqrtf( e.m_position.x * e.m_position.x + e.m_position.y * e.m_position.y );
		Vec2 rectMin = Vec2( posXY - e.m_radius, 0.f );
		Vec2 rectMax = Vec2( posXY + e.m_radius, e.m_height );


		//if( DoesLineAndRectangleIntersect( start, result.m_forwardNormal, maxDistance, rectMin, rectMax ) )
		//{
		//	result.m_didImpact = true;
		//
		//	float distToEntity = GetDistanceSquared2D( start, e.m_position );
		//	if( distToEntity < nearestDistSquared )
		//	{
		//		nearestDistSquared = distToEntity;
		//		result.m_impactEntity = m_allEntities[i];
		//	}
		//}
	}

	
	return result;
}

//-------------------------------------------------------------------------------------------------------------
RaycastResult TileMap::RaycastAgainstCeilingAndFloor( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance, float ceilingHeight )
{
	RaycastResult result;
	result.m_startPosition = start;
	result.m_forwardNormal = forwardDirection.GetNormalized();
	result.m_maxDistance = maxDistance;

	if( start.y > ceilingHeight || start.x < 0.f )
	{
		return result;
	}

	Vec2 endPos = start + result.m_forwardNormal * maxDistance;
	if( endPos.y > ceilingHeight )
	{
		// Hit Ceiling
		result.m_didImpact = true;
		result.m_impactFraction = ( ceilingHeight - start.y ) / ( endPos.y - start.y );
	}
	else if( endPos.y < 0.f )
	{
		// Hit Floor
		result.m_didImpact = true;
		result.m_impactFraction = ( start.y - 0.f ) / ( start.y - endPos.y );
	}

	// #ToDo: Check if impactPosition is needed here
	//result.m_impactPosition = start + result.m_impactFraction * result.m_forwardNormal;
	return result;
}

bool TileMap::DoesRayAndAABB2Intersect( const Ray& r, const Box3& box, float& t )
{
	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (box.bounds[r.sign[0]].x - r.orig.x) * r.invdir.x;
	tmax = (box.bounds[1-r.sign[0]].x - r.orig.x) * r.invdir.x;
	tymin = (box.bounds[r.sign[1]].y - r.orig.y) * r.invdir.y;
	tymax = (box.bounds[1-r.sign[1]].y - r.orig.y) * r.invdir.y;

	if( (tmin > tymax) || (tymin > tmax) )
		return false;

	if( tymin > tmin )
		tmin = tymin;
	if( tymax < tmax )
		tmax = tymax;

	tzmin = (box.bounds[r.sign[2]].z - r.orig.z) * r.invdir.z;
	tzmax = (box.bounds[1-r.sign[2]].z - r.orig.z) * r.invdir.z;

	if( (tmin > tzmax) || (tzmin > tmax) )
		return false;

	if( tzmin > tmin )
		tmin = tzmin;
	if( tzmax < tmax )
		tmax = tzmax;

	t = tmin;

	if( t < 0 ) 
	{
		t = tmax;
		if( t < 0 ) { return false; }
	}

	return true;
}


//-------------------------------------------------------------------------------------------------------------
void TileMap::CreateTiles( XmlElement const& mapDef )
{
	// Parse Dimensions
	m_tileDimensions = ParseXmlAttribute( mapDef, "dimensions", m_tileDimensions );
	if( m_tileDimensions == IntVec2::ZERO )
	{
		g_theConsole->Error( "%s map has invalid dimensions.", m_mapName.c_str() );
	}

	// Calculate number of Tiles
	m_numTiles = m_tileDimensions.x * m_tileDimensions.y;

	m_tiles.reserve( m_numTiles ); 

	int tileIndex = 0;
	for( int tileY = 0; tileY < m_tileDimensions.y; ++tileY )
	{
		for( int tileX = 0; tileX < m_tileDimensions.x; ++tileX )
		{
			MapTile tile;
			tile.m_tileCoords = IntVec2( tileX, tileY );
			m_tiles.push_back( tile );
			++ tileIndex;
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
void TileMap::PopulateTiles( XmlElement const& mapDef )
{
	std::map< char, MapRegionType const* >	legend;
	ParseLegend( legend, mapDef );
	ParseMapRows( legend, mapDef );
	//ParseEntities( legend, mapDef );
}

//-------------------------------------------------------------------------------------------------------------
void TileMap::PopulateEntities( XmlElement const& mapDef )
{
	XmlElement const* entitiesDef = mapDef.FirstChildElement( "Entities" );
	for( XmlElement const* element = entitiesDef->FirstChildElement(); element; element = element->NextSiblingElement() )
	{
		if( element->DoesElementNameEqual( "PlayerStart" ) )
		{
			Vec2 playerStartPos2D = ParseXmlAttribute( *element, "pos", Vec2::ZERO );
			m_playerStartPos = Vec3( playerStartPos2D.x, playerStartPos2D.y, m_playerStartPos.z );
			m_playerStartyaw = ParseXmlAttribute( *element, "yaw", m_playerStartyaw );

			// #ToDo: Create the actual player 
			Entity* player = SpawnNewEntityOfType( "Marine" );
			player->m_isPlayer = true;
			player->m_position = playerStartPos2D;
			player->m_yawDegrees = m_playerStartyaw;
			player->m_canBePushedByEntities = false;

			//g_theGame->m_player = player;
		}
		else if( element->DoesElementNameEqual( "Actor" ) )
		{
			std::string typeName = ParseXmlAttribute( *element, "type", "" );
			if( typeName == "" )
			{
				g_theConsole->Error( "ERROR: Failed to create actor type %s", typeName.c_str() );
				return;
			}
			Entity* actor = SpawnNewEntityOfType( typeName );
			
			//actor->m_isNPC = true;
			actor->m_position = ParseXmlAttribute( *element, "pos", Vec2::ZERO );
			actor->m_yawDegrees = ParseXmlAttribute( *element, "yaw", 0.f );
		}
		else if( element->DoesElementNameEqual( "Portal" ) )
		{
			std::string typeName = ParseXmlAttribute( *element, "name", "" );
			if( typeName == "" )
			{
				g_theConsole->Error( "ERROR: Failed to create portal type %s", typeName.c_str() );
				return;
			}

			Entity* newEntity = SpawnNewEntityOfType( typeName );
			Portal* portal = dynamic_cast<Portal*>( newEntity );
			if( !portal )
			{
				g_theConsole->Error( "ERROR: Failed to create portal type %s", typeName.c_str() );
				return;
			}
			portal->m_position = ParseXmlAttribute( *element, "pos", Vec2::ZERO );
			portal->m_yawDegrees = ParseXmlAttribute( *element, "yaw", 0.f );
			portal->m_destMapStr = ParseXmlAttribute( *element, "destMap", "" );
			portal->m_destPos = ParseXmlAttribute( *element, "destPos", Vec2::ZERO );
			portal->m_destYawOffset = ParseXmlAttribute( *element, "destYawOffset", portal->m_destYawOffset );
		}
	}
}
