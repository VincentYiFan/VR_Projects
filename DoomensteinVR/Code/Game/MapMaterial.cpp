#include "Game/MapMaterial.hpp"
#include "Game/GameCommon.hpp"

//------------------------------------------------------------------------------------------------------
STATIC std::map< std::string, MapMaterial* >		MapMaterial::s_materials; 
STATIC std::map< std::string, MaterialsSheet* >		MaterialsSheet::s_materialsSheets;

STATIC	MapMaterial const* MapMaterial::s_defaultMaterial = nullptr;
//------------------------------------------------------------------------------------------------------

MaterialsSheet const* MaterialsSheet::GetDefinition( std::string const& defName )
{
	return s_materialsSheets[ defName ];
}

void MaterialsSheet::GetUVsForSpriteCoords( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, IntVec2 spriteCoords ) const
{
	float uEachSpriteGridX = ( 1.f / m_layout.x ); 
	float vEachSpriteGridY = ( 1.f / m_layout.y ); 

	float uAtMinX = uEachSpriteGridX * spriteCoords.x;
	float uAtMaxX = uAtMinX + uEachSpriteGridX;
	float vAtMaxY = 1.f - vEachSpriteGridY * spriteCoords.y;
	float vAtMinY = vAtMaxY - vEachSpriteGridY;

	out_uvAtMins = Vec2( uAtMinX, vAtMinY );
	out_uvAtMaxs = Vec2( uAtMaxX, vAtMaxY );
}

Texture const* MaterialsSheet::GetDiffuseTexture() const
{
	return m_diffuseTexture;
}

MaterialsSheet::MaterialsSheet( XmlElement const& materialsDef )
{
	m_name = ParseXmlAttribute( materialsDef, "name", m_name );
	m_layout = ParseXmlAttribute( materialsDef, "layout", m_layout );

	XmlElement const* diffuseDef = materialsDef.FirstChildElement( "Diffuse" );
	std::string imagePath = ParseXmlAttribute( *diffuseDef, "image", "" );
	
	// #ToDo: check error
	//Texture const* spriteSheetTexture  = g_theRenderer->CreateOrGetTextureFromFile( imagePath.c_str() );
	m_diffuseTexture = g_theRenderer->CreateOrGetTextureFromFile( imagePath.c_str() );
	m_diffuseSheet = new SpriteSheet( *m_diffuseTexture, m_layout );
}

SpriteSheet* MaterialsSheet::CreateMaterialsSpriteSheetFromElement( XmlElement const& materialsDef )
{
	UNUSED( materialsDef );
	return nullptr;
}


//------------------------------------------------------------------------------------------------------
STATIC void MapMaterial::LoadDefinitions( const char* dataFilePath )
{
	char const* requiredRootElementName = "MapMaterialTypes";
	XmlDocument mapMaterialsDocs;
	mapMaterialsDocs.LoadFile( dataFilePath );
	if( mapMaterialsDocs.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		g_theConsole->Error( "Failed to load MapMaterial definitions data file: %s", dataFilePath );
		return;
	}

	XmlElement* root = mapMaterialsDocs.RootElement();
	if( !root )
	{
		g_theConsole->Error( "MapMaterial definitions file: %s lacked a root XML node", dataFilePath );
		return;
	}

	if( !root->DoesElementNameEqual( requiredRootElementName ) )
	{
		g_theConsole->Error( "MapMaterial definitions file: %s had root element <%s> - which required to be <%s>", dataFilePath, root->Name(), requiredRootElementName );
		return;
	}

	std::string defaultMaterialName = ParseXmlAttribute( *root, "default", "" );
	if( defaultMaterialName == "" )
	{
		g_theConsole->Error( "MapMaterial definitions file: %s lacked a default material name in root XML node", dataFilePath );
		return;
	}

	for( XmlElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement() )
	{
		if( element->DoesElementNameEqual( "MaterialsSheet" ) )
		{
			MaterialsSheet* newMaterialsSheet = new MaterialsSheet( *element );
			auto found = MaterialsSheet::s_materialsSheets.find( newMaterialsSheet->m_name );
			if( found != MaterialsSheet::s_materialsSheets.end() )
			{
				g_theConsole->Error( "ERROR: Multiple conflicting definitions for MaterialsSheet" );
				delete newMaterialsSheet;
				delete found->second;
				found->second = nullptr;
				continue;
			}

			MaterialsSheet::s_materialsSheets[ newMaterialsSheet->m_name ] = newMaterialsSheet;
			//if( !g_defaultMaterialsSheet )
			//{
			//	g_defaultMaterialsSheet = newMaterialsSheet;
			//}
		}
		else if( element->DoesElementNameEqual( "MaterialType" ) )
		{
			MapMaterial* newMaterial = new MapMaterial( *element );
			auto found = s_materials.find( newMaterial->m_name );
			if( found != s_materials.end() )
			{
				g_theConsole->Error( "ERROR: Multiple conflicting definitions for MapMaterials" );
				delete newMaterial;
				delete found->second;
				found->second = nullptr;
				continue;
			}

			s_materials[ newMaterial->m_name ] = newMaterial;
		}
		else 
		{
			// #ToDo
			g_theConsole->Error( "Illegal root-child element found." );
		}
	}

	s_defaultMaterial = MapMaterial::GetDefinition( defaultMaterialName );
	if( s_defaultMaterial == nullptr )
	{
		g_theConsole->Error( "ERROR: Failed to create default map material in <%s>", dataFilePath );
	}
}

MapMaterial::MapMaterial( XmlElement const& materialDef )
{
	m_name = ParseXmlAttribute( materialDef, "name", "" );
	std::string sprtieSheetName = ParseXmlAttribute( materialDef, "sheet", "" );
	m_spriteSheet = MaterialsSheet::GetDefinition( sprtieSheetName );
	m_spriteCoords = ParseXmlAttribute( materialDef, "spriteCoords", m_spriteCoords );
	m_spriteSheet->GetUVsForSpriteCoords( m_uvAtMins, m_uvAtMaxs, m_spriteCoords );
}

STATIC MapMaterial const* MapMaterial::GetDefinition( std::string const& defName )
{
	// #ToDo
	auto found = s_materials.find( defName );
	if( found == s_materials.end() )
	{
		return nullptr;
	}
	return found->second;
}
