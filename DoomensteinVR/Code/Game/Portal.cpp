#include "Game/Portal.hpp"
#include "Game/Map.hpp"

Portal::Portal( EntityDef const& entityDef, Map* map )
	:Entity( entityDef, map )
{
	m_canBePushedByEntities = false;
}

Portal::~Portal()
{
}

void Portal::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	for( int i = 0; i < (int)m_map->m_allEntities.size(); ++i )
	{
		if( m_map->DoEntitiesOverlap( *this, *m_map->m_allEntities[i] ) && m_map->m_allEntities[i]->IsPlayer() )
		{
			// Teleport the player to the destination map
			Map* destMap = nullptr;
			if( m_destMapStr == "" )
			{
				destMap = g_theGame->m_theWorld->m_currentMap;
			}
			else
			{
				destMap = g_theGame->m_theWorld->GetMap( m_destMapStr.c_str() );
			}
			
			g_theGame->m_theWorld->EnterMap( destMap, Vec3( m_destPos, 0.f ), m_map->m_allEntities[i]->m_yawDegrees + m_destYawOffset );
		}
	}
}

void Portal::Render( Camera& camera )
{
	SpriteDefinition spriteDef = m_spriteSheet->GetSpriteDefinition( 0 );
	Vec2 uvMins, uvMaxs;
	spriteDef.GetUVs( uvMins, uvMaxs );

	Vec3 centerPos = Vec3( m_position.x, m_position.y, 0.f ) + Vec3( 0.f, 0.f, 0.5f * m_spriteSize.y );
	Vec3 BL, BR, TR, TL;
	GetBillboardQuad( BL, BR, TR, TL, camera, centerPos, m_spriteSize, m_billboardMode );
	Rgba8 tint = Rgba8::WHITE;

	// Bind Texture
	g_theRenderer->BindTexture( &spriteDef.GetTexture() );

	std::vector<Vertex_PCU> verts;
	verts.emplace_back( Vec3( BR.x, BR.y, BR.z ), tint, Vec2( uvMins.x, uvMins.y ) );
	verts.emplace_back( Vec3( BL.x, BL.y, BL.z ), tint, Vec2( uvMaxs.x, uvMins.y ) );
	verts.emplace_back( Vec3( TL.x, TL.y, TL.z ), tint, Vec2( uvMaxs.x, uvMaxs.y ) );

	verts.emplace_back( Vec3( BR.x, BR.y, BR.z ), tint, Vec2( uvMins.x, uvMins.y ) );
	verts.emplace_back( Vec3( TL.x, TL.y, TL.z ), tint, Vec2( uvMaxs.x, uvMaxs.y ) );
	verts.emplace_back( Vec3( TR.x, TR.y, TR.z ), tint, Vec2( uvMins.x, uvMaxs.y ) );

	g_theRenderer->DrawVertexArray( verts );
}
