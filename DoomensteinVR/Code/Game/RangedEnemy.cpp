#include "Game/RangedEnemy.hpp"
#include "Game/Projectile.hpp"
#include "Game/TileMap.hpp"
#include "Game/GameCommon.hpp"
#include "Game/LighthouseTracking.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

RangedEnemy::RangedEnemy( EntityDef const& entityDef, Map* map )
	:Entity( entityDef, map )
{
	m_isNPC = true;
	m_faction = Faction::EVIL;
	m_attackCoolDownTimer.SetSeconds( 0.f );
}

RangedEnemy::~RangedEnemy()
{
}

void RangedEnemy::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );

	Entity* player = g_theGame->GetPlayer();
	if( player == nullptr ) {
		return;
	}

	switch ( m_state )
	{
	case AIState::IDLE: {
		constexpr float maxVisionRange = 5.0f;
		bool hasSeenPlayer = IsPointInForwardSector2D( player->m_position, m_position, m_yawDegrees, 80.f, maxVisionRange );
		if( hasSeenPlayer )
		{
			m_state = AIState::ATTACK;
		}
	}
	break;


	case AIState::ATTACK: {
		// Turn toward player
		Vec2 dispToPlayer = (player->m_position - m_position).GetNormalized();
		float goalAngle = Atan2Degrees( dispToPlayer.y, dispToPlayer.x );

		while( fabsf( m_yawDegrees - goalAngle ) > 5.f )
		{
			m_yawDegrees = GetTurnedToward( m_yawDegrees, goalAngle, 180.f * deltaSeconds );
		}

		if( m_attackCoolDownTimer.HasElapsed() )
		{
			SpawnProjectile();
			m_attackCoolDownTimer.SetSeconds( RANGED_ENEMY_ATTACK_CD );
		}
	}
	break;

	case AIState::DEAD: {
		m_deathTimerCount += deltaSeconds;

		// Delete the entity after 3 secs
		if( m_deathTimerCount > 3.f ) {
			m_isReadyToBeDeleted = true;
		}
	}
	break;

	}
}

void RangedEnemy::Render( Camera& camera )
{
	Anim* animation = m_anims.at( "Idle" );
	SpriteDefinition const* spriteDef = nullptr;

	Vec2 dispToCam = Vec2( camera.m_transform.m_position.x, camera.m_transform.m_position.y ) - m_position;
	Vec2 dispToCamerLocal = dispToCam.GetRotatedDegrees( -m_yawDegrees );
	float largestDotProduct = -999999.f;
	int spriteIndex = 0;

	switch( m_state ) {
	case AIState::IDLE:				animation = m_anims.at( "Idle" );		break;
	case AIState::PATROL:			animation = m_anims.at( "Walk" );		break;
	case AIState::ATTACK:			animation = m_anims.at( "Attack" );		break;
	case AIState::HURT:				animation = m_anims.at( "Pain" );		break;
	case AIState::DEAD:				animation = m_anims.at( "Death" );		break;
	default: g_theConsole->Error( "Invalid AI State!" );
	}

	int n = 0;
	for( int i = 0; i < (int) animation->m_animsAtAngles.size(); ++i )
	{
		if( DotProduct2D( dispToCamerLocal, animation->m_animsAtAngles[i].m_idealNormal ) > largestDotProduct )
		{
			spriteIndex = animation->m_animsAtAngles[i].m_spriteIndexes[0];
			largestDotProduct = DotProduct2D( dispToCamerLocal, animation->m_animsAtAngles[i].m_idealNormal );
			n = i;
		}
	}

	SpriteAnimDefinition atkAnimDef = SpriteAnimDefinition( *m_spriteSheet, animation->m_animsAtAngles[n].m_spriteIndexes, RANGED_ENEMY_ATTACK_CD, SpriteAnimPlaybackType::LOOP );
	SpriteAnimDefinition deadAnimDef = SpriteAnimDefinition( *m_spriteSheet, animation->m_animsAtAngles[n].m_spriteIndexes, 1.f, SpriteAnimPlaybackType::ONCE );
	SpriteAnimDefinition defaultAnimDef = SpriteAnimDefinition( *m_spriteSheet, animation->m_animsAtAngles[n].m_spriteIndexes, 1.f, SpriteAnimPlaybackType::LOOP );

	if( m_state == AIState::ATTACK )
	{
		spriteDef = &atkAnimDef.GetSpriteDefAtTime( m_lifeTime );
	}
	else if( m_state == AIState::DEAD )
	{
		spriteDef = &deadAnimDef.GetSpriteDefAtTime( m_deathTimerCount );
	}
	else
	{
		spriteDef = &deadAnimDef.GetSpriteDefAtTime( m_lifeTime );
	}



	Vec2 uvMins, uvMaxs;
	spriteDef->GetUVs( uvMins, uvMaxs );

	Vec3 centerPos = Vec3( m_position.x, m_position.y, 0 ) + Vec3( 0.f, 0.f, 0.5f * m_spriteSize.y );
	Vec3 BL, BR, TR, TL;
	GetBillboardQuad( BL, BR, TR, TL, camera, centerPos, m_spriteSize, m_billboardMode );

	Rgba8 tint = Rgba8::WHITE;

	Vec3 tangent, bitangent, normal;
	CalculateTBN( tangent, bitangent, normal, BR-BL, TL-BL );
	std::vector<Vertex_PCUTBN> verts;
	verts.emplace_back( Vec3( BR.x, BR.y, BR.z ), tint, Vec2( uvMins.x, uvMins.y ), tangent, bitangent, normal );
	verts.emplace_back( Vec3( BL.x, BL.y, BL.z ), tint, Vec2( uvMaxs.x, uvMins.y ), tangent, bitangent, normal );
	verts.emplace_back( Vec3( TL.x, TL.y, TL.z ), tint, Vec2( uvMaxs.x, uvMaxs.y ), tangent, bitangent, normal );

	verts.emplace_back( Vec3( BR.x, BR.y, BR.z ), tint, Vec2( uvMins.x, uvMins.y ), tangent, bitangent, normal );
	verts.emplace_back( Vec3( TL.x, TL.y, TL.z ), tint, Vec2( uvMaxs.x, uvMaxs.y ), tangent, bitangent, normal );
	verts.emplace_back( Vec3( TR.x, TR.y, TR.z ), tint, Vec2( uvMins.x, uvMaxs.y ), tangent, bitangent, normal );

	g_theRenderer->BindTexture( &spriteDef->GetTexture() );

	GPUMesh mesh = GPUMesh( g_theRenderer );
	mesh.UpdateVertices( verts );
	g_theRenderer->DrawMesh( &mesh );
	
	spriteDef = nullptr;
}

void RangedEnemy::SpawnProjectile()
{
	TileMap* tileMap = dynamic_cast<TileMap*>(g_theGame->m_theWorld->m_currentMap);
	Projectile* projectile = dynamic_cast<Projectile*>(tileMap->SpawnNewEntityOfType( "Plasma Bolt" ));
	if( projectile != nullptr ) {
		Vec3 spawnLocation = GetProjectileSpawnLocation();
		projectile->m_position = Vec2( spawnLocation.x, spawnLocation.y );
		projectile->m_height = spawnLocation.z;

		Vec3 playerPos = (g_theGame->m_worldCameraLeft.m_transform.m_position + g_theGame->m_worldCameraRight.m_transform.m_position) / 2.f;
		playerPos.z = /*g_theLighthouse->GetHMDPosition().z*/ 1.5f;

		//Vec2 forwardDir2D = Vec2( playerPos.x - m_position.x, playerPos.y - m_position.y ).GetNormalized();
		//Vec2 forwardDir2D( GetForwardVector() );	// #ToDo: Bullet should be able to shoot vertically
		//Vec3 forwardDir3D( forwardDir2D, 0.f );
		Vec3 forwardDir3D = Vec3( playerPos - spawnLocation ).GetNormalized();
		projectile->SetVelocity( forwardDir3D * BULLET_SPEED );
		projectile->SetFaction( Faction::EVIL );
	}
}

Vec3 RangedEnemy::GetProjectileSpawnLocation() const
{
	Vec2 rightDisp = Vec2( 0.f, -1.f ).GetRotatedDegrees( m_yawDegrees );
	return Vec3( m_position, 0.9f ) + Vec3( rightDisp * 0.35f, 0.f );
}