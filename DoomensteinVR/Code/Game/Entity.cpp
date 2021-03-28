#include "Game/Entity.hpp"
#include "Game/TileMap.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

Entity::Entity( EntityDef const& entityDef, Map* map )
{
	m_map = map;

	m_radius				= entityDef.m_physicsRadius;
	m_height				= entityDef.m_height;
	m_eyeHeight				= entityDef.m_eyeHeight;
	m_walkSpeed				= entityDef.m_walkSpeed;
	m_spriteSize			= entityDef.m_spriteSize;
	m_billboardMode			= entityDef.m_billboardMode;
	m_spriteSheet			= entityDef.m_spriteSheet;
	m_spriteSheetFilePath	= entityDef.m_spriteSheetFilePath;
	m_spriteSheetLayout		= entityDef.m_spriteSheetLayout;
	m_anims					= entityDef.m_anims;
}

Entity::~Entity()
{
	
}

void Entity::Update( float deltaSeconds )
{	
	// Update the Entity's Life Time
	m_lifeTime += deltaSeconds;

	if( m_health <= 0 ) 
	{
		m_isDead = true;
		m_state = AIState::DEAD;
	}
}

void Entity::Render( Camera& camera )
{
	if( IsPlayer() )
	{
		return;
	}

	Anim* walkAnim = m_anims.at( "Walk" );
	Vec2 dispToCam = Vec2( camera.m_transform.m_position.x, camera.m_transform.m_position.y ) - m_position;
	Vec2 dispToCamerLocal = dispToCam.GetRotatedDegrees( -m_yawDegrees );
	float largestDotProduct = -9999.f;
	int spriteIndex = 0;
	for( int i = 0; i < walkAnim->m_animsAtAngles.size(); ++i )
	{
		if( DotProduct2D( dispToCamerLocal, walkAnim->m_animsAtAngles[i].m_idealNormal ) > largestDotProduct )
		{
			spriteIndex = walkAnim->m_animsAtAngles[i].m_spriteIndexes[ 0 ];
			largestDotProduct = DotProduct2D( dispToCamerLocal, walkAnim->m_animsAtAngles[i].m_idealNormal );

			//m_walkAnimDef = new SpriteAnimDefinition( *m_spriteSheet, walkAnim->m_animsAtAngles[i].m_spriteIndexes, 1.15f, SpriteAnimPlaybackType::LOOP );
		}
	}

	
	//SpriteDefinition spriteDef = m_walkAnimDef->GetSpriteDefAtTime( m_lifeTime );

	SpriteDefinition spriteDef = m_spriteSheet->GetSpriteDefinition( spriteIndex );
	Vec2 uvMins, uvMaxs;
	spriteDef.GetUVs( uvMins, uvMaxs );

	Vec3 centerPos = Vec3( m_position.x, m_position.y, 0.f ) + Vec3( 0.f, 0.f, 0.5f * m_spriteSize.y );
	Vec3 BL, BR, TR, TL;
	GetBillboardQuad( BL, BR, TR, TL, camera, centerPos, m_spriteSize, m_billboardMode );
	Rgba8 tint = Rgba8::WHITE;

	// Bind Texture
	g_theRenderer->BindTexture( &spriteDef.GetTexture() );
	
	Vec3 tangent, bitangent, normal;
	CalculateTBN( tangent, bitangent, normal, BR-BL, TL-BL );

	std::vector<Vertex_PCUTBN> verts;
	verts.emplace_back( Vec3( BR.x, BR.y, BR.z ), tint, Vec2( uvMins.x, uvMins.y ), tangent, bitangent, normal );
	verts.emplace_back( Vec3( BL.x, BL.y, BL.z ), tint, Vec2( uvMaxs.x, uvMins.y ), tangent, bitangent, normal );
	verts.emplace_back( Vec3( TL.x, TL.y, TL.z ), tint, Vec2( uvMaxs.x, uvMaxs.y ), tangent, bitangent, normal );

	verts.emplace_back( Vec3( BR.x, BR.y, BR.z ), tint, Vec2( uvMins.x, uvMins.y ), tangent, bitangent, normal );
	verts.emplace_back( Vec3( TL.x, TL.y, TL.z ), tint, Vec2( uvMaxs.x, uvMaxs.y ), tangent, bitangent, normal );
	verts.emplace_back( Vec3( TR.x, TR.y, TR.z ), tint, Vec2( uvMins.x, uvMaxs.y ), tangent, bitangent, normal );

	GPUMesh mesh = GPUMesh( g_theRenderer );
	mesh.UpdateVertices( verts );
	g_theRenderer->DrawMesh( &mesh );
}

void Entity::DebugRender( Camera& camera ) const
{
	if( IsPlayer() ) {
		return;
	}

	if( !g_theGame->GetIsDebugRenderingActive() ) {
		return;
	}

	Vec3 startPos = Vec3( m_position.x, m_position.y, 0.f );
	Vec3 centerPos = startPos + Vec3( 0.f, 0.f, 0.5f * m_spriteSize.y );
	Vec3 BL, BR, TR, TL;
	GetBillboardQuad( BL, BR, TR, TL, camera, centerPos, m_spriteSize, m_billboardMode );

	g_theRenderer->BindTexture( nullptr );
	g_theDebugRenderSystem->DebugAddWorldLine( TL, Rgba8::MAGENTA, TR, Rgba8::CYAN, 0.01f, 0.f );
	g_theDebugRenderSystem->DebugAddWorldLine( TR, Rgba8::MAGENTA, BR, Rgba8::CYAN, 0.01f, 0.f );
	g_theDebugRenderSystem->DebugAddWorldLine( BR, Rgba8::MAGENTA, BL, Rgba8::CYAN, 0.01f, 0.f );
	g_theDebugRenderSystem->DebugAddWorldLine( BL, Rgba8::MAGENTA, TL, Rgba8::CYAN, 0.01f, 0.f );

	// Render cylinder
	float thickness = m_radius * 2.f;
	g_theDebugRenderSystem->DebugAddWorldLine( startPos, Rgba8::CYAN, startPos + Vec3( 0.f, 0.f, m_height ), Rgba8::CYAN, thickness, 0.f, true );

	// Render forward vector
	Vec3 eyeEndPositon = GetEyePosition() + 0.25f * Vec3( GetForwardVector().x, GetForwardVector().y, 0.f );
	g_theDebugRenderSystem->DebugAddWorldArrow( GetEyePosition(), Rgba8::MAGENTA, eyeEndPositon, Rgba8::MAGENTA, 0.f, 0.02f );	
}

void Entity::AddDebugDrawVertsToMesh( Mesh_PCT& mesh, Camera const& camera ) const
{
	UNUSED( mesh );
	UNUSED( camera );
}

FloatRange Entity::GetZRange() const
{
	return FloatRange();
}

Vec3 Entity::GetEyePosition() const
{
	Vec3 eyePosition;
	Vec2 forward = GetForwardVector();
	eyePosition = Vec3( m_position.x, m_position.y, 0.f ) + Vec3( 0.f, 0.f, m_eyeHeight ) + (Vec3( forward.x, forward.y, 0.f ) * m_radius);
	return eyePosition;
}

void Entity::SetIsPlayer( bool isPlayer )
{
	m_isPlayer = isPlayer;
}

void Entity::SetFaction( Faction faction )
{
	m_faction = faction;
}

Vec2 Entity::GetForwardVector() const
{
	Vec2 forward = Vec2( 1.f, 0.f );
	forward = forward.GetRotatedDegrees( m_yawDegrees ).GetNormalized();
	return forward;
}

void Entity::TakeDamage( int damage )
{
	m_health -= damage;
}








