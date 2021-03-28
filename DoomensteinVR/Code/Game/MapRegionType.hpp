#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include <string>
#include <map>

class MapMaterial;

class MapRegionType
{
public:
	static void						LoadDefinitions( const char* dataFilePath );
	static MapRegionType const*		GetDefinitions( std::string defName );
	static MapRegionType const*		GetDefaultRegionType()	{ return s_defaultRegionType; }
	MapMaterial const*				GetSideMaterial() const		{ return m_sideMaterial; }
	MapMaterial const*				GetFloorMaterial() const	{ return m_floorMaterial; }
	MapMaterial const*				GetCeilingMaterial() const	{ return m_ceilingMaterial; }


	bool							IsSolid() const		{ return m_isSolid; }

private:
	MapRegionType( XmlElement const& materialDef );
	//MapMaterial const* ParseMapMaterialFromChildElement( XmlElement const& parent, char const* mapMaterialName );

private:
	static std::map< std::string, MapRegionType* >	s_regionTypes;
	static MapRegionType const*						s_defaultRegionType;


private:
	std::string				m_name;
	bool					m_isSolid			= true;
	MapMaterial const*		m_sideMaterial		= nullptr; 
	MapMaterial const*		m_floorMaterial		= nullptr; 
	MapMaterial const*		m_ceilingMaterial	= nullptr; 
};
