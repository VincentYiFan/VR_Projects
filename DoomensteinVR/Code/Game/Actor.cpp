#include "Game/Actor.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Game/LighthouseTracking.hpp"

Actor::Actor( EntityDef const& entityDef, Map* map )
	:Entity( entityDef, map )
{
	m_isNPC = true;
	m_faction = Faction::EVIL;
	m_attackCoolDownTimer.SetSeconds( 0.f );
	m_hurtTimer.SetSeconds( 0.f );
}

Actor::~Actor()
{
	if( m_walkAnimDef )
	{
		delete m_walkAnimDef;
		m_walkAnimDef = nullptr;
	}

	if( m_idleAnimDef )
	{
		delete m_idleAnimDef;
		m_idleAnimDef = nullptr;
	}
}

void Actor::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );

	if( IsPlayer() ) {
		return;
	}

	Entity* player = g_theGame->GetPlayer();

	switch( m_state ) {
	case AIState::IDLE: 
	{
		// Debug Draw Sector
		if( g_theGame->GetIsDebugRenderingActive() )
		{
			Vec3 p0 = Vec3( m_position.x, m_position.y, m_eyeHeight );
			Vec3 disp1 = Vec3( GetForwardVector() * PINKY_MAX_VISION_RANGE, 0.f ).GetRotatedAboutZDegrees( -37.5f );
			Vec3 disp2 = Vec3( GetForwardVector() * PINKY_MAX_VISION_RANGE, 0.f ).GetRotatedAboutZDegrees( 37.5f );
			Vec3 p1 = p0 + disp1;
			Vec3 p2 = p0 + disp2;

			g_theDebugRenderSystem->DebugAddWorldLine( p0, Rgba8::RED, p1, Rgba8::RED, 0.1f );
			g_theDebugRenderSystem->DebugAddWorldLine( p0, Rgba8::RED, p2, Rgba8::RED, 0.02f );
			g_theDebugRenderSystem->DebugAddWorldLine( p1, Rgba8::RED, p2, Rgba8::RED, 0.02f );
		}

		bool hasSeenPlayer = IsPointInForwardSector2D( player->m_position, m_position, m_yawDegrees, 80.f, PINKY_MAX_VISION_RANGE );
		if( hasSeenPlayer )
		{
			m_state = AIState::CHASE_TARGET;
		}
	}
	break;
	
	case AIState::CHASE_TARGET:
	{
		Vec2 dispToPlayer = ( player->m_position - m_position ).GetNormalized();

		// Turn toward player
		float goalAngle = Atan2Degrees( dispToPlayer.y, dispToPlayer.x );
		
		while( fabsf( m_yawDegrees - goalAngle ) > 5.f )
		{
			m_yawDegrees = GetTurnedToward( m_yawDegrees, goalAngle, m_turningSpeed * deltaSeconds );
		}

		// Move toward player
		m_position += GetForwardVector() * m_walkSpeed * deltaSeconds;

		if( GetDistance2D( player->m_position, m_position ) < 1.5f )
		{
			m_state = AIState::ATTACK;
		}
	}
	break;

	case AIState::HURT:
	{
		if( m_hurtTimer.HasElapsed() )
		{
			m_state = AIState::CHASE_TARGET;
		}
	}
	break;
	
	case AIState::ATTACK:
	{
		if( GetDistanceSquared2D( player->m_position, m_position ) > 2.f )
		{
			m_attackTimer = 0.f;
			m_flyingHeight = 0.f;
			m_attackCoolDownTimer.SetSeconds( 0.f );
			m_hasAttackDamageApplied = false;
			m_state = AIState::CHASE_TARGET;
		}

		if( m_attackCoolDownTimer.HasElapsed() )
		{
			// Update the attack timer
			m_attackTimer += deltaSeconds;
		}

		if( m_attackTimer > 0.5f && m_attackTimer < 1.25f ) {
			//m_flyingHeight = Interpolate( 0.f, 0.75f, m_attackTimer / 2.f ); 
			//m_flyingHeight = 0.7f;
			//m_position += GetForwardVector() * ( m_walkSpeed * 1.5f ) * deltaSeconds;
		}
		else {
			//m_flyingHeight = 0.f;
		}

		if( m_attackTimer > 0.9f && m_attackTimer < 2.f && !m_hasAttackDamageApplied )
		{
			// Attack
			if( player && !player->IsDead() )
			{
				bool doesIntersectWithPlayer = DoDiscsOverlap( player->m_position, player->m_radius, m_position, m_radius + 1.f );
				if( doesIntersectWithPlayer )
				{
					player->TakeDamage( 10 );
					m_hasAttackDamageApplied = true;

					// Apply pulse on the controllers
					if( g_theLighthouse && g_theLighthouse->IsValid() )
					{

						g_theLighthouse->AddHapticPulse( vr::TrackedControllerRole_LeftHand, 0.1f );
						g_theLighthouse->AddHapticPulse( vr::TrackedControllerRole_RightHand, 0.1f );
					}
				}		
			}
		}
		else if( m_attackTimer > 2.f )
		{
			m_attackTimer = 0.f;
			m_hasAttackDamageApplied = false;
			m_attackCoolDownTimer.SetSeconds( PINKY_ATTACK_COOLDOWN );
		}
	}
	break;

	case AIState::DEAD: {
		m_deadTimerCount += deltaSeconds;

		// temp
		m_isReadyToBeDeleted = true;
	}

	}
}

void Actor::Render( Camera& camera )
{
	if( IsPlayer() ) {	// In VR, we don't render the player
		return;
	}

	Anim* animation = nullptr;
	switch( m_state ) {
		case AIState::IDLE:				animation = m_anims.at( "Idle" );		break;
		case AIState::CHASE_TARGET:		animation = m_anims.at( "Walk" );		break;
		case AIState::ATTACK:			animation = m_anims.at( "Walk" );		break;
		case AIState::HURT:				animation = m_anims.at( "Walk" );		break;
		case AIState::DEAD:				animation = m_anims.at( "Death" );		break;
		default: g_theConsole->Error( "Invalid AI State!" );
	}

	Vec2 dispToCam = Vec2( camera.m_transform.m_position.x, camera.m_transform.m_position.y ) - m_position;
	Vec2 dispToCamerLocal = dispToCam.GetRotatedDegrees( -m_yawDegrees );
	float largestDotProduct = -9999.f;
	int spriteIndex = 0;
	int angleIndex;

	for( int i = 0; i < animation->m_animsAtAngles.size(); ++i )
	{
		if( DotProduct2D( dispToCamerLocal, animation->m_animsAtAngles[i].m_idealNormal ) > largestDotProduct )
		{
			spriteIndex = animation->m_animsAtAngles[i].m_spriteIndexes[0];
			largestDotProduct = DotProduct2D( dispToCamerLocal, animation->m_animsAtAngles[i].m_idealNormal );
			angleIndex = i;
		}
	}

	SpriteAnimDefinition attackAnimDef = SpriteAnimDefinition( *m_spriteSheet, animation->m_animsAtAngles[angleIndex].m_spriteIndexes, 1.15f, SpriteAnimPlaybackType::LOOP );
	SpriteAnimDefinition deadAnimDef = SpriteAnimDefinition( *m_spriteSheet, animation->m_animsAtAngles[angleIndex].m_spriteIndexes, 1.f, SpriteAnimPlaybackType::ONCE );
	SpriteAnimDefinition defaultAnimDef = SpriteAnimDefinition( *m_spriteSheet, animation->m_animsAtAngles[angleIndex].m_spriteIndexes, 1.15f, SpriteAnimPlaybackType::LOOP );

	Vec2 uvMins, uvMaxs;
	if( m_state == AIState::DEAD )
	{
		SpriteDefinition spriteDef = deadAnimDef.GetSpriteDefAtTime( m_deadTimerCount );
		spriteDef.GetUVs( uvMins, uvMaxs );
		g_theRenderer->BindTexture( &spriteDef.GetTexture() );
	}
	else {
		SpriteDefinition spriteDef = defaultAnimDef.GetSpriteDefAtTime( m_lifeTime );
		spriteDef.GetUVs( uvMins, uvMaxs );
		g_theRenderer->BindTexture( &spriteDef.GetTexture() );
	}

	
	Vec3 centerPos = Vec3( m_position.x, m_position.y, m_flyingHeight ) + Vec3( 0.f, 0.f, 0.5f * m_spriteSize.y );
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

	// Bind Texture
	//g_theRenderer->BindTexture( &spriteDef->GetTexture() );

	GPUMesh mesh = GPUMesh( g_theRenderer );
	mesh.UpdateVertices( verts );
	g_theRenderer->DrawMesh( &mesh );
}

void Actor::SetAIState( AIState state )
{
	m_state = state;
}
