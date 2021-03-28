#pragma once
#include "Game/Game.hpp" 
#include "Game/EntityDef.hpp"

//-------------------------------------------------------------------------------------------------------------
class Camera;
//-------------------------------------------------------------------------------------------------------------
typedef std::vector<Vertex_PCU> Mesh_PCT;
//-------------------------------------------------------------------------------------------------------------
enum class Faction
{
	DEFAULT_FACTION = -1,	

	GOOD,
	EVIL,
	NEUTRAL
};
//-------------------------------------------------------------------------------------------------------------
enum class AIState
{
	IDLE,
	PATROL,
	CHASE_TARGET,
	ATTACK,
	HURT,
	DEAD
};
//-------------------------------------------------------------------------------------------------------------

class Entity 
{
	friend class Map;

protected:
	Entity( EntityDef const& entityDef, Map* map );

public:
	virtual ~Entity();
	virtual void		Update( float deltaSeconds );
	virtual void		Render( Camera& camera );
	virtual void		DebugRender( Camera& camera ) const;
	virtual void		TakeDamage( int damage );
	virtual bool		IsDead() const					{ return m_isDead; }
	virtual bool		IsReadyToBeDeleted() const		{ return m_isReadyToBeDeleted; }
	virtual bool		IsPlayer() const				{ return m_isPlayer; }
	virtual bool		IsProjectile() const			{ return m_isProjectile; }
	virtual bool		IsNPC() const					{ return m_isNPC; }
	virtual bool		PushesEntities() const			{ return m_canPushEntities; }
	virtual bool		IsPushedByEntities() const		{ return m_canBePushedByEntities; }
	virtual bool		IsPushedByWalls() const			{ return m_canBePushedByWalls; }
	virtual void		AddDebugDrawVertsToMesh( Mesh_PCT& mesh, Camera const& camera ) const;
	virtual FloatRange	GetZRange() const;
	virtual Vec3		GetEyePosition() const;
	virtual Vec2		GetForwardVector() const;
	virtual void		SetIsPlayer( bool isPlayer );
	virtual void		SetFaction( Faction faction );

public:
	float				m_lifeTime = 0.f;

	Vec2				m_position;
	int					m_health = 100;
	float				m_yawDegrees = 0.f;
	bool				m_isDead = false;
	bool				m_isReadyToBeDeleted = false; 
	bool				m_isPlayer = false;
	bool				m_isProjectile = false;
	bool				m_isNPC = false;
	Faction				m_faction = Faction::DEFAULT_FACTION;

	// Physics Properties
	bool				m_canBePushedByWalls	= true;
	bool				m_canBePushedByEntities = true;
	bool				m_canPushEntities		= true;
	float				m_mass = 1.f;
	float				m_radius = 0.f;
	float				m_height = 0.f;
	float				m_eyeHeight = 0.f;
	float				m_walkSpeed = 0.f;

	// Appearance
	Vec2				m_spriteSize = Vec2::ZERO;
	SpriteSheet*		m_spriteSheet = nullptr;
	eBillboardMode		m_billboardMode = eBillboardMode::BILLBOARD_MODE_CAMERA_FACING_XYZ;
	std::string			m_spriteSheetFilePath;
	IntVec2				m_spriteSheetLayout	= IntVec2::ZERO;
	std::map< std::string, Anim* >	m_anims;

	AIState				m_state = AIState::IDLE;

protected:
	Map*				m_map  = nullptr;
};