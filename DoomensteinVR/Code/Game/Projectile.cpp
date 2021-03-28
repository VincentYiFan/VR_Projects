#include "Game/Projectile.hpp"
#include "Game/TileMap.hpp"
#include "Game/GameCommon.hpp"
#include "Game/LighthouseTracking.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include <vector>

Projectile::Projectile( EntityDef const& entityDef, Map* map )
	:Entity( entityDef, map )
{
	m_isProjectile = true;
	m_canBePushedByWalls = false;
	m_canBePushedByEntities = false;
	m_canPushEntities = false;

	m_transform.SetPosition( Vec3( m_position, m_height ) );
	m_transform.SetScale( Vec3( 0.0025f, 0.0025f, 0.0025f ) );
	m_transform.SetYawDegrees( m_yawDegrees - 90.f );

	m_mesh = g_theGame->m_bulletMesh;
}

Projectile::~Projectile()
{
	m_mesh = nullptr;
}

void Projectile::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );

	// Update the position
	m_position += Vec2( m_velocity.x, m_velocity.y ) * deltaSeconds;
	m_transform.SetPosition( Vec3( m_position, m_height ) );

	// Update the height
	m_height += m_velocity.z * deltaSeconds;

	// Update the yaw degrees
	m_yawDegrees = Vec2( m_velocity.x, m_velocity.y ).GetAngleDegrees();
	m_transform.SetYawDegrees( m_yawDegrees - 90.f );
	//m_transform.m_orientationYaw = m_yawDegrees;

	// Check overlap events
	CheckCollision();
}

void Projectile::Render( Camera& camera )
{
	UNUSED( camera );

	if( m_faction == Faction::GOOD && m_mesh )
	{
		g_theRenderer->SetModelMatrix( m_transform.ToMatrix() );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Textures/Bullet.png" ) );
		g_theRenderer->DrawMesh( m_mesh );
	}
	else if( m_faction == Faction::EVIL && m_mesh )
	{
		g_theRenderer->SetModelMatrix( m_transform.ToMatrix() );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Textures/Bullet.png" ) );
		g_theRenderer->DrawMesh( m_mesh );
	}
}

void Projectile::DebugRender( Camera& camera ) const
{
	UNUSED( camera );
}

void Projectile::CheckCollision()
{
	// Check if the projectile hit a wall or go out of the bounds
	TileMap* tileMap = dynamic_cast<TileMap*>(m_map);
	if( tileMap ) 
	{
		IntVec2 tileCoords = tileMap->GetTileCoordsForWorldPosition( m_position );
		if( tileMap->IsTileSolid( tileCoords ) ) 
		{
			// Mark as dead
			m_isDead = true;
		}

		// Debug render bounds
		Vec3 mins = Vec3( m_position.x, m_position.y, m_height ) - Vec3( 0.025f, 0.025f, 0.025f );
		Vec3 maxs = Vec3( m_position.x, m_position.y, m_height ) + Vec3( 0.025f, 0.025f, 0.025f );
		AABB3 bulletBounds( mins, maxs );

		if( g_theGame->m_isDebugRenderingActive ) {		
			g_theDebugRenderSystem->DebugAddWorldWireBounds( bulletBounds, Rgba8::MAGENTA, 0.f, true );
		}
		
		// Check if the hit entity
		if( m_faction == Faction::EVIL ) 
		{
			Vec3 playerPos = (g_theGame->m_worldCameraLeft.m_transform.m_position + g_theGame->m_worldCameraRight.m_transform.m_position) / 2.f;

			Vec3 player_mins = Vec3( playerPos.x - 0.4f, playerPos.y - 0.4f, 0.f );
			Vec3 player_maxs = Vec3(  playerPos.x + 0.4f, playerPos.y + 0.4f, /*playerPos.z*/ 1.75f );
			AABB3 playerBounds( player_mins, player_maxs );
			/*g_theDebugRenderSystem->DebugAddWorldWireBounds( playerBounds, Rgba8::CYAN, 0.f, true );*/

			if( DoAABB3sOverlap3D( bulletBounds, playerBounds ) && !m_hasAppliedDamage ) {
				// Apply damage
				g_theGame->GetPlayer()->TakeDamage( 10 );
				m_hasAppliedDamage = true;
				m_isDead = true;	// Mark self as dead

				g_theLighthouse->AddHapticPulse( vr::TrackedControllerRole_LeftHand, 0.25f );
				g_theLighthouse->AddHapticPulse( vr::TrackedControllerRole_RightHand, 0.25f );
			}
		}
		
	}	
}

void Projectile::SetVelocity( Vec3 velocity )
{
	m_velocity = velocity;
}
