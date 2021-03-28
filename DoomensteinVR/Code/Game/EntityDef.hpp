#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Game/GameCommon.hpp"
#include <map>
#include <string>

class SpriteSheet;

//---------------------------------------------------------------------------------------------------------
struct AnimAtAngle
{
	AnimAtAngle( XmlAttribute const& animViewAttribute, std::string const& angleName );

	Vec2				m_idealNormal; // show this AnimAtAngle when looking most directly at this normal (in local space)
	std::vector<int>	m_spriteIndexes; // animation sequence to play out of our sprite sheet	
};


//---------------------------------------------------------------------------------------------------------
struct Anim
{
	Anim( XmlElement const& animElement );

	std::vector< AnimAtAngle > m_animsAtAngles;
};


//---------------------------------------------------------------------------------------------------------
class EntityDef
{
public:
	static void					LoadDefinitions( const char* dataFilePath );
	static EntityDef const*		GetDefinitions( std::string const& defName );

	static std::map< std::string, EntityDef* >	s_entityTypes;

private:
	EntityDef( XmlElement const& entityDef );

public:
	std::string						m_className;
	std::string						m_typeName;
	SpriteSheet*					m_spriteSheet				= nullptr;
	std::map< std::string, Anim* >	m_anims;
	eBillboardMode					m_billboardMode				= eBillboardMode::BILLBOARD_MODE_CAMERA_FACING_XYZ;
	float							m_physicsRadius				= 0.30f;
	float							m_height					= 0.60f;
	float							m_eyeHeight					= 0.55f;
	float							m_walkSpeed					= 0.f;
	Vec2							m_spriteSize				= Vec2::ONE;
	bool							m_canPushEntities			= false;
	bool							m_canBePushedByEntities		= false;
	bool							m_canBePushedByWalls		= false;
	float							m_mass						= 1.f;

	std::string						m_spriteSheetFilePath;
	IntVec2							m_spriteSheetLayout			= IntVec2::ZERO;

	// Projectile Properties
	float							m_speed						= 0.f;
	IntRange						m_attackDamageRange			= IntRange( 0, 0 );
};
