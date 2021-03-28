#pragma once
#include "Game/Map.hpp"
#include "Game/MapRegionType.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <vector>

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Forward declaration
struct AABB3;
class MapRegionType;
class GPUMesh;
struct Vertex_PCUTBN;
//struct Vertex_PCU;
//-----------------------------------------------------------------------------------------------------------------------------------------------
//typedef std::vector<Vertex_PCU> Mesh_PCT;

struct Ray
{
public:
	Ray( const Vec3& orig, const Vec3& dir ) 
		: orig( orig ), dir( dir )
	{
		invdir = Vec3( 1.f/dir.x, 1.f/dir.y, 1.f/dir.z );
		sign[0] = ( invdir.x < 0 );
		sign[1] = ( invdir.y < 0 );
		sign[2] = ( invdir.z < 0 );
	}

public:
	Vec3 orig, dir;
	Vec3 invdir;
	int sign[3];
};

class Box3
{
public:
	Box3( const Vec3& vmin, const Vec3& vmax )
	{
		bounds[0] = vmin;
		bounds[1] = vmax;
	}
	Vec3 bounds[2];
};

class MapTile
{
private:
	friend class TileMap;

	bool	IsSolid() const	{ return m_type->IsSolid(); }

	IntVec2					m_tileCoords = IntVec2::ZERO;
	MapRegionType const*	m_type = nullptr;
};


class TileMap : public Map
{
public:
	TileMap( char const* mapName, XmlElement const& mapDef );
	~TileMap();

	virtual void	Update( float deltaSeconds ) override;
	virtual void	UpdateMeshes() override;
	virtual void	Render( Camera& camera ) const override;
	virtual void	PushEntityOutOfWalls( Entity& e ) override;
	void			PushEntityOutOfTileIfSolid( Entity& e, IntVec2 const& tileCoords );
	int				GetTileIndexForTileCoords( int tileX, int tileY ) const;
	bool			IsTileSolid( IntVec2 const& tileCoords ) const;
	AABB3			Get3DBoundsForTile( IntVec2 tileCoords ) const;
	AABB2			Get2DBoundsForTile( IntVec2 tileCoords ) const;
	IntVec2			GetTileCoordsForWorldPosition( Vec2 const& worldPosition );

	// Raycast
	virtual RaycastResult	Raycast( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance ) override;
			RaycastResult	RaycastAgainstEntities( Vec3 const& start, Vec3 const& forwardDirection, float maxDistance );
			RaycastResult	RaycastAgainstEntities2D( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance );
			RaycastResult	RaycastAgainstEntities2D_Updated( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance );
			RaycastResult	RaycastAgainstEntitiesZ( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance );
			RaycastResult	RaycastAgainstCeilingAndFloor( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance, float ceilingHeight = 1.f );

	bool			DoesRayAndAABB2Intersect( const Ray& r, const Box3& box, float& t );

protected:
	void			CreateTiles( XmlElement const& mapDef );
	void			PopulateTiles( XmlElement const& mapDef );
	void			PopulateEntities( XmlElement const& mapDef );
	void			ParseLegend( std::map< char, MapRegionType const* >& legend, XmlElement const& mapDef );
	void			ParseMapRows( std::map< char, MapRegionType const* >& legend, XmlElement const& mapDef );
	//void			ParseEntities( std::map< char, MapRegionType const* >& legend, XmlElement const& mapDef );

	void			AddVertsForTile( GPUMesh* mesh, int tileIndex );
	//void			AddVertsForTile( Mesh_PCT& mesh, int tileIndex ) const;
	void			AddVertsForSolidTile( GPUMesh* mesh, MapTile const& tile );
	//void			AddVertsForSolidTile( Mesh_PCT& mesh, MapTile const& tile ) const;
	void			AddVertsForOpenTile( GPUMesh* mesh, MapTile const& tile );
	//void			AddVertsForOpenTile( Mesh_PCT& mesh, MapTile const& tile ) const;


private:
	IntVec2					m_tileDimensions = IntVec2::ZERO;
	std::vector<MapTile>	m_tiles;
	int						m_numTiles = 0;
	std::string				m_mapRowsStr;

public:
	//Mesh_PCT				m_worldMesh;
	GPUMesh* m_worldMesh = nullptr;
	std::vector<Vertex_PCUTBN> m_vertices;
	std::vector<uint> m_indices;
};

