#include "Game/World.hpp"
#include "Game/TileMap.hpp"
#include "Game/GameCommon.hpp"
#include "Game/LighthouseTracking.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/GameObject.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/FileUtils.hpp"

World::World( Camera& cameraLeft, Camera& cameraRight )
	:m_cameraLeft( cameraLeft )
	,m_cameraRight( cameraRight )
{
	CreateMaps();

	std::string startMapName = g_gameConfigBlackboard.GetValue( "startMap", "EmptyRoom" );
	EnterMap( GetMap( startMapName.c_str() ) );
}

World::~World()
{
}

void World::Update( float deltaSeconds )
{
	m_currentMap->Update( deltaSeconds );
}

void World::Render( Camera& camera ) const
{ 
	if( m_currentMap )
	{
		m_currentMap->UpdateMeshes();
		m_currentMap->Render( camera );
	}
}

void World::Render( eCameras cameraToRender ) const
{
	switch( cameraToRender )
	{
	case eCameras::LEFT_CAMERA:
	{
		Render( m_cameraLeft );
	}
	break;
	
	case eCameras::RIGHT_CAMERA:
	{
		Render( m_cameraRight );
	}
	break;

	default:
		g_theConsole->Error( "%s - Unknown camera to render!", __FUNCTION__ ); 
	}
}


void World::EnterMap( Map* map )
{
	EnterMap( map, map->m_playerStartPos, map->m_playerStartyaw );
}

void World::EnterMap( Map* map, Vec3 startPos, float startYaw )
{
	m_currentMap = map;

	Entity* player = map->m_allEntities[0];
	player->m_position = Vec2( startPos.x, startPos.y );
	player->m_yawDegrees = startYaw;

	// Close DevConsole
	g_theConsole->SetIsOpen( false );
}

Map* World::GetMap( char const* mapName )
{
	return m_maps[ mapName ];
}

void World::CreateMaps()
{
	std::string mapFolder = "Data/Maps/";
	Strings mapFileNames = GetFileNamesInFolder( mapFolder, "*.xml" );
	std::string str = Stringf("Loading %i map files from %s", mapFileNames.size(), mapFolder.c_str() );
	g_theConsole->PrintString( Rgba8::WHITE, str );

	for( int i = 0; i < (int)mapFileNames.size(); ++i )
	{
		std::string const& mapFileName = mapFileNames[i];
		std::string mapFilePath = mapFolder + mapFileName;
		const char* defXmlFilePath = mapFilePath.c_str();

		XmlDocument xmlDocument;
		xmlDocument.LoadFile( defXmlFilePath );

		if( xmlDocument.ErrorID() != tinyxml2::XML_SUCCESS )
		{
			g_theConsole->Error( "Failed to load: %s", defXmlFilePath );
			return;
		}

		XmlElement* rootElement = xmlDocument.RootElement();
		if( !rootElement )
		{
			g_theConsole->Error( "Failed to find root element in: %s", defXmlFilePath );
			return;
		}

		std::string typeName = ParseXmlAttribute( *rootElement, "type", "Unknown" );
		if( typeName == "TileMap" )
		{
			// Split by '/'
			Strings splitUpPath = SplitStringOnDelimiter( defXmlFilePath, '/' );

			// Split by '.'
			Strings spiltUpMapName = SplitStringOnDelimiter( splitUpPath[2], '.' );

			char const* mapName = spiltUpMapName[0].c_str();
			g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Loading map %s...", mapName ) );

			TileMap* tileMap = new TileMap( mapName, *rootElement );
			m_maps[mapName] = tileMap;
		}
	}
}


COMMAND( warp, "map,pos,yaw" )
{
	std::string mapName = args.GetValue( "map", "" );
	Vec2 pos = args.GetValue( "pos", Vec2::ZERO );
	float yawDegrees = args.GetValue( "yaw", 0.f );

	if( mapName.length() > 0 )	// map name is not NULL
	{
		Map* newMap = g_theGame->m_theWorld->GetMap( mapName.c_str() );

		if( newMap != nullptr ) // Found the map
		{
			if( pos != Vec2::ZERO && yawDegrees != 0.f )
			{
				g_theGame->m_theWorld->EnterMap( newMap, Vec3( pos.x, pos.y, 0.65f ), yawDegrees );
			}
			else
			{
				// General case
				g_theGame->m_theWorld->EnterMap( newMap );
			}
			//g_theInput->SetCursorMode( MOUSE_MODE_RELATIVE );
			g_theConsole->PrintString( Rgba8::CYAN, Stringf( "Map is now: %s", mapName.c_str() ) );
		}
		else
		{
			g_theConsole->Error( "Failed to find map %s", mapName.c_str() );
		}
	}
}

