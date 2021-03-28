#pragma once
#include <map>
#include <string>

//----------------------------------------------------------------------------------
class Camera;
class Map;
struct Vec3;
//----------------------------------------------------------------------------------

enum class eCameras
{
	LEFT_CAMERA,
	RIGHT_CAMERA
};

class World
{
public:
	World( Camera& cameraLeft, Camera& cameraRight );
	~World();

	void Update( float deltaSeconds );
	void Render( Camera& camera ) const;
	void Render( eCameras cameraToRender ) const;


public:
	void EnterMap( Map* map );
	void EnterMap( Map* map, Vec3 startPos, float startYaw );
	Map* GetMap( char const* mapName );
	void CreateMaps();


public:
	std::map< std::string, Map*> m_maps;
	Map*		m_currentMap = nullptr;
	Camera&		m_cameraLeft;
	Camera&     m_cameraRight;
};


