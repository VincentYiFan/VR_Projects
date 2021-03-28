#include "Game/EntityDef.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

STATIC std::map< std::string, EntityDef* >	EntityDef::s_entityTypes;

Vec2 GetLocalNormalForAngleName( const std::string& angleName )
{
	if( angleName == "front" )			return Vec2( 1.f, 0.f );
	if( angleName == "frontLeft" )		return Vec2( 1.f, 1.f ).GetNormalized();
	if( angleName == "left" )			return Vec2( 0.f, 1.f );
	if( angleName == "backLeft" )		return Vec2( -1.f, 1.f ).GetNormalized();
	if( angleName == "back" )			return Vec2( -1.f, 0.f );
	if( angleName == "backRight" )		return Vec2( -1.f, -1.f ).GetNormalized();
	if( angleName == "right" )			return Vec2( 0.f, -1.f );
	if( angleName == "frontRight" )		return Vec2( 1.f, -1.f ).GetNormalized();

	g_theConsole->Error( "ERROR: Failed to get local normal for angle name %s", angleName.c_str() );
	return Vec2::ZERO;
}

void EntityDef::LoadDefinitions( const char* dataFilePath )
{
	g_theConsole->PrintString( Rgba8::WHITE, "Parsing entity types from: " + std::string( dataFilePath ) );

	char const* requiredRootElementName = "EntityTypes";
	XmlDocument entityTypesDocs;
	entityTypesDocs.LoadFile( dataFilePath );
	if( entityTypesDocs.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		g_theConsole->Error( "ERROR: Failed to load entity type from %s", dataFilePath );
		return;
	}

	XmlElement* root = entityTypesDocs.RootElement();
	if( !root )
	{
		g_theConsole->Error( "ERROR: Failed to find root element from %s", dataFilePath );
		return;
	}

	if( !root->DoesElementNameEqual( requiredRootElementName ) )
	{
		g_theConsole->Error( "ERROR: Root element is called %s, must be %s", root->Name(), requiredRootElementName );
		return;
	}

	for( XmlElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement() )
	{
		if( !( element->DoesElementNameEqual( "Actor" ) || element->DoesElementNameEqual( "Entity" ) || element->DoesElementNameEqual( "Projectile" )|| element->DoesElementNameEqual( "Portal" ) ) )
		{
			g_theConsole->Error( "ERROR: ChildElement is called %s, must be %s, %s or %s", element->Name(), "Actor", "Entity", "Projectile" );
			return;
		}
		EntityDef* newEntityDef = new EntityDef( *element );
		s_entityTypes[ newEntityDef->m_typeName ] = newEntityDef;
	}
}

STATIC EntityDef const* EntityDef::GetDefinitions( std::string const& defName )
{
	return s_entityTypes[ defName ];
}

EntityDef::EntityDef( XmlElement const& entityDef )
{
	m_className = entityDef.Name();
	m_typeName = ParseXmlAttribute( entityDef, "name", "" );
	if( m_className == "" )
	{
		g_theConsole->Error( "ERROR: Invalid EntityType class name for %s", entityDef.Name() );
		return;
	}

	for( XmlElement const* def = entityDef.FirstChildElement(); def; def = def->NextSiblingElement() )
	{
		if( def->DoesElementNameEqual( "Physics" ) )
		{
			m_physicsRadius	= ParseXmlAttribute( *def, "radius",	m_physicsRadius );
			m_height		= ParseXmlAttribute( *def, "height",	m_height );
			m_eyeHeight		= ParseXmlAttribute( *def, "eyeHeight", m_eyeHeight );
			m_walkSpeed		= ParseXmlAttribute( *def, "walkSpeed", m_walkSpeed );
			m_speed			= ParseXmlAttribute( *def, "speed",		m_speed );
		}
		else if( def->DoesElementNameEqual( "Appearance" ) )
		{
			m_spriteSize = ParseXmlAttribute( *def, "size", m_spriteSize );
			std::string billboardModeName = ParseXmlAttribute( *def, "billboard", "CameraFacingXY" );
			m_billboardMode = GetBillboardModeFromName( billboardModeName );

			m_spriteSheetFilePath = ParseXmlAttribute( *def, "spriteSheet", "" );

			if( m_spriteSheetFilePath == "" )
			{
				// Not a fatal Error
				g_theConsole->Error( "Can't found spriteSheet for %s: %s", m_className.c_str(), m_typeName.c_str() );
				return;
			}

			m_spriteSheetLayout = ParseXmlAttribute( *def, "layout", m_spriteSheetLayout );
			if( m_spriteSheetLayout == IntVec2::ZERO )
			{
				g_theConsole->Error( "Invalid spriteSheet layout for %s: %s", m_className.c_str(), m_typeName.c_str() );
				return;
			}

			Texture* spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile( m_spriteSheetFilePath.c_str() );
			m_spriteSheet = new SpriteSheet( *spriteSheetTexture, m_spriteSheetLayout );
			//if( !spriteSheetTexture )
			//{
			//	g_theConsole->Error( "ERROR:Failed to create texture for %s: %s", m_className.c_str(), m_typeName.c_str() );
			//}

			// Parse Anims
			for( XmlElement const* animElement = def->FirstChildElement(); animElement; animElement = animElement->NextSiblingElement() )
			{
				Anim* newAnim = new Anim( *animElement );
 				std::string animName = animElement->Name();
				m_anims[ animName ] = newAnim;
			}
		}
		else if( def->DoesElementNameEqual( "Gameplay" ) )
		{
			m_attackDamageRange = ParseXmlAttribute( *def, "damage", m_attackDamageRange );
		}
		else
		{
			// #ToDo: Error message
		}
	}
}


AnimAtAngle::AnimAtAngle( XmlAttribute const& animViewAttribute, std::string const& angleName )
{
	m_idealNormal = GetLocalNormalForAngleName( angleName );

	std::string spriteIndexesStr = animViewAttribute.Value();
	Strings spriteIndexes = SplitStringOnDelimiter( spriteIndexesStr, ',' );
	for( std::string spriteIndexStr : spriteIndexes )
	{
		m_spriteIndexes.push_back( std::stoi( spriteIndexStr ) );
	}
}

Anim::Anim( XmlElement const& animElement )
{
	for( XmlAttribute const* animAtAngleAtt = animElement.FirstAttribute(); animAtAngleAtt; animAtAngleAtt = animAtAngleAtt->Next() )
	{
		AnimAtAngle animAtAngle = AnimAtAngle( *animAtAngleAtt, animAtAngleAtt->Name() );
		m_animsAtAngles.push_back( animAtAngle );
	}
}
