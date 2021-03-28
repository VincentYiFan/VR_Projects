#include "Game/MapRegionType.hpp"
#include "Game/MapMaterial.hpp"
#include "Engine/Core/EngineCommon.hpp"

STATIC std::map< std::string, MapRegionType* > MapRegionType::s_regionTypes;
STATIC MapRegionType const* MapRegionType::s_defaultRegionType=  /*MapRegionType::s_regionTypes[ "Default" ]*/ nullptr;

MapRegionType::MapRegionType( XmlElement const& mapRegionTypeDef )
{
	m_name = ParseXmlAttribute( mapRegionTypeDef, "name", "" );
	m_isSolid = ParseXmlAttribute( mapRegionTypeDef, "isSolid", m_isSolid );
}


STATIC void MapRegionType::LoadDefinitions( const char* dataFilePath )
{
	g_theConsole->PrintString( Rgba8::WHITE, "Parsing map region types from: " + std::string( dataFilePath ) );

	char const* requiredRootElementName = "MapRegionTypes";
	XmlDocument mapRegionDocs;
	mapRegionDocs.LoadFile( dataFilePath );
	if( mapRegionDocs.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		g_theConsole->Error( "ERROR: Failed to load map region type from %s", dataFilePath );
		return;
	}

	XmlElement* root = mapRegionDocs.RootElement();
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

	std::string defaultRegionTypeName = ParseXmlAttribute( *root, "default", "" );
	if( defaultRegionTypeName == "" )
	{
		g_theConsole->Error( "ERROR: Failed to find default region type" );
		return;
	}

	for( XmlElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement() )
	{
		if( !element->DoesElementNameEqual( "RegionType" ) )
		{
			g_theConsole->Error( "ERROR: ChildElement is called %s, must be %s", element->Name(), "RegionType" );
			return;
		}
		MapRegionType* newMapRegionType = new MapRegionType( *element );
		
		// temp
		for( XmlElement* matDef = element->FirstChildElement(); matDef; matDef = matDef->NextSiblingElement() )
		{
			std::string matName = ParseXmlAttribute( *matDef, "material", "" );
			if( matDef->DoesElementNameEqual( "Floor" ) )
			{
				newMapRegionType->m_floorMaterial = MapMaterial::GetDefinition( matName );
			}
			else if( matDef->DoesElementNameEqual( "Ceiling" ) )
			{
				newMapRegionType->m_ceilingMaterial = MapMaterial::GetDefinition( matName );
			}
			else if( matDef->DoesElementNameEqual( "Side" ) )
			{
				newMapRegionType->m_sideMaterial = MapMaterial::GetDefinition( matName );
			}
			else 
			{
				// #ToDo: Error message
			}
		}

		s_regionTypes[ newMapRegionType->m_name ] = newMapRegionType;
	}

 	s_defaultRegionType = GetDefinitions( defaultRegionTypeName );
	// #ToDo: validate this is in fact a region type
}

STATIC MapRegionType const* MapRegionType::GetDefinitions( std::string defName )
{
	auto found = s_regionTypes.find( defName );
	if( found == s_regionTypes.end() )
	{
		return nullptr;
	}
	return found->second;
}
 