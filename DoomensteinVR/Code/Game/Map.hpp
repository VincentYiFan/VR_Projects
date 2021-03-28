#pragma once
#include "Game/Entity.hpp"
#include "Game/RaycastResult.hpp"
#include "Engine/Math/Vec3.hpp"
#include <string>
#include <vector>


//----------------------------------------------------------------------------
typedef std::vector<Entity*> EntityList;

//----------------------------------------------------------------------------

class Map
{
public:
	Map( char const* mapName );
	virtual ~Map();

	virtual void	Update( float deltaSeconds ) = 0;
	virtual void	Render( Camera& camera ) const = 0;
	virtual void	UpdateMeshes() = 0;

	// Entity Management
	virtual Entity* SpawnNewEntityOfType( std::string const& typeName );
	virtual Entity* SpawnNewEntityOfType( EntityDef const& type );
	virtual void	RemoveEntityFromMap( Entity* e );
	virtual void	AddEntityToMap( Entity* e );
	virtual void	RemoveEntityFromList( Entity* e, EntityList& list );
	virtual void	AddEntityToList( Entity* e, EntityList& list ); 

	// Entity Physics
	virtual void	ResolveEntityCollision();
	virtual void	PushEntityVsEntity(Entity& a, Entity& b );
	virtual void	PushEntityOutOfWalls( Entity& e ) = 0;
	virtual void	PushEntitiesOffEachOther( Entity& a, Entity& b );
	virtual void	PushMobileEntityOffImmobileEntiity( Entity& mobile, Entity& immobile ); 
	virtual bool	DoEntitiesOverlap( Entity& a, Entity& b );

	// Raycast
	virtual RaycastResult	Raycast( Vec2 const& start, Vec2 const& forwardDirection, float maxDistance ) = 0;

public:
	std::string		m_mapName;
	Vec3			m_playerStartPos = Vec3( 1.5f, 1.5f, 2.f ); // was z = 0.65f
	float			m_playerStartyaw = 0.f;
	EntityList		m_allEntities;
	EntityList		m_NPCs;	// non-Player Actors only (does not include players)
	EntityList		m_projectiles;
	EntityList		m_players;
};


