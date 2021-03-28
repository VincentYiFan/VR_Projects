#include "Game/Map.hpp"
#include "Game/TileMap.hpp"
#include "Game/EntityDef.hpp"
#include "Game/Actor.hpp"
#include "Game/RangedEnemy.hpp"
#include "Game/Projectile.hpp"
#include "Game/Portal.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Map::Map( char const* mapName )
{
	m_mapName = mapName;
}

Map::~Map()
{

}

Entity* Map::SpawnNewEntityOfType( std::string const& typeName )
{
	EntityDef const* entityDef = EntityDef::GetDefinitions( typeName );
	if( entityDef )
	{
		return SpawnNewEntityOfType( *entityDef );
	}
	else 
	{
		g_theConsole->Error( "Failed to create enemy of unknown type \"%s\"! Must be one of: \n", typeName.c_str() );
		auto typeIter = EntityDef::s_entityTypes.begin();
		for( ; typeIter != EntityDef::s_entityTypes.end(); ++typeIter )
		{
			g_theConsole->PrintString( Rgba8::RED, Stringf( "  %s\n", typeIter->first.c_str() ) );
		}
		return nullptr;
	}
}

Entity* Map::SpawnNewEntityOfType( EntityDef const& type )
{
	Entity* newEntity = nullptr;

	if( type.m_className == "Actor" )
	{
		if( type.m_typeName == "Pinky" )
		{
			newEntity = new Actor( type, this );
		}
		else if( type.m_typeName == "RangedEnemy" )
		{
			newEntity = new RangedEnemy( type, this );
		}
		else
		{
			// #ToDo: Replace marine with a actually player class instead of using marine. 
			if( type.m_typeName == "Marine" ) {
				newEntity = new Actor( type, this );
			}
			else { g_theConsole->Error( "Error-Unknown entity type: %s", type.m_typeName.c_str() ); }
		}
	}
	else if( type.m_className == "Projectile" )
	{
		newEntity = new Projectile( type, this );
	}
	else if( type.m_className == "Portal" )
	{
		newEntity = new Portal( type, this );
	}
	else if( type.m_className == "Entity" )
	{
		newEntity = new Entity( type, this );
	}
	else
	{
		ERROR_AND_DIE( "ERROR: Unknow entity type to spawn" );
	}

	AddEntityToMap( newEntity );
	return newEntity;
}

void Map::RemoveEntityFromMap( Entity* e )
{
	RemoveEntityFromList( e, m_allEntities );
	RemoveEntityFromList( e, m_players );
	RemoveEntityFromList( e, m_projectiles );
	RemoveEntityFromList( e, m_NPCs );
}

void Map::AddEntityToMap( Entity* e )
{
	if( !e ) {
		return; 
	}

	AddEntityToList( e, m_allEntities );
	if( e->IsPlayer() )
	{
		AddEntityToList( e, m_players );
	}
	else if( e->IsProjectile() )
	{
		AddEntityToList( e, m_projectiles );
	}
	else if( e->IsNPC() )
	{
		AddEntityToList( e, m_NPCs );
	}
}

void Map::RemoveEntityFromList( Entity* e, EntityList& list )
{
	for( int i = 0; i < (int)list.size(); ++i )
	{
		if( e == list[i] )
		{
			list.erase( list.begin() + i );
		}
	}
}

void Map::AddEntityToList( Entity* e, EntityList& list )
{
	list.push_back( e );
}

void Map::PushEntitiesOffEachOther( Entity& a, Entity& b )
{
	PushDiscsOutOfEachOther2D( a.m_position, a.m_radius, b.m_position, b.m_radius );
}

void Map::PushMobileEntityOffImmobileEntiity( Entity& mobile, Entity& immobile )
{
	PushDiscOutOfDisc2D( mobile.m_position, mobile.m_radius, immobile.m_position, immobile.m_radius );
}

void Map::ResolveEntityCollision()
{
	for( int i = 0; i < m_allEntities.size(); ++i )
	{
		for( int j = 0; j < m_allEntities.size(); ++j )
		{
			// Avoid resolving the same entity and make sure they are both valid
			if( m_allEntities[i] && m_allEntities[j] && (m_allEntities[i] != m_allEntities[j]) )
			{
				PushEntityVsEntity( *m_allEntities[i], *m_allEntities[j] );
			}
		}
	}
}

void Map::PushEntityVsEntity( Entity& a, Entity& b )
{
	if( !a.IsPushedByEntities() && !b.IsPushedByEntities() )
	{
		return;
	}
	if( !a.PushesEntities() && !b.PushesEntities() )
	{
		return;
	}
	if( !DoEntitiesOverlap( a, b ) )
	{
		return;
	}
	if( a.PushesEntities() && b.IsPushedByEntities() && !a.IsPushedByEntities() ) // A pushes B (only)
	{
		//PushDiscOutOfDisc2D( b.m_position, b.m_radius, a.m_position, a.m_radius );
		PushMobileEntityOffImmobileEntiity( b, a );
	}
	else if( b.PushesEntities() && a.IsPushedByEntities() && !b.IsPushedByEntities() ) // B pushes A (only)
	{
		//PushDiscOutOfDisc2D( a.m_position, a.m_radius, b.m_position, b.m_radius );
		PushMobileEntityOffImmobileEntiity( a, b );
	}
	else if( a.PushesEntities() && b.PushesEntities() && a.IsPushedByEntities() && b.IsPushedByEntities() )
	{
		/*PushDiscsOutOfEachOther2D( a.m_position, a.m_radius, b.m_position, b.m_radius );*/
		PushEntitiesOffEachOther( a, b );
	}
}

bool Map::DoEntitiesOverlap( Entity& a, Entity& b )
{
	return DoDiscsOverlap( a.m_position, a.m_radius, b.m_position, b.m_radius );
}
