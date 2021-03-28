#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include <string>
#include <map>


class MaterialsSheet
{
public:
	static	MaterialsSheet const*	GetDefinition( std::string const& defName );

	void			GetUVsForSpriteCoords( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, IntVec2 spriteCoords ) const;
	Texture const*	GetDiffuseTexture() const;
	//Texture const*	GetNormalsTexture() const;
	//Texture const*	GetSpecularTexture() const;
	//Texture const*	GetEmissiveexture() const;

private:
	friend class MapMaterial; 

	MaterialsSheet( XmlElement const& materialsSheetDef );
	SpriteSheet*	CreateMaterialsSpriteSheetFromElement( XmlElement const& materialsDef );

	static std::map< std::string, MaterialsSheet* >		s_materialsSheets;


	std::string			m_name;
	IntVec2				m_layout = IntVec2::ONE;
	Texture const*		m_diffuseTexture	= nullptr;
	SpriteSheet*		m_diffuseSheet		= nullptr;
	//SpriteSheet*		m_normalSheet		= nullptr;
	//SpriteSheet*		m_specularSheet		= nullptr;
	//SpriteSheet*		m_emissiveSheet		= nullptr;
};

class MapMaterial
{
public:
	static void						LoadDefinitions( const char* dataFilePath );
	static MapMaterial const*		GetDefinition( std::string const& defName );
	Vec2							GetUVAtMins() const		{ return m_uvAtMins; }
	Vec2							GetUVAtMaxs() const		{ return m_uvAtMaxs; }
	static MapMaterial const*		GetDefaultMaterial()	{ return s_defaultMaterial; }

private:
	MapMaterial( XmlElement const& materialDef );

private:
	static std::map< std::string, MapMaterial* >	s_materials; 
	static MapMaterial const*						s_defaultMaterial;

private:
	std::string				m_name;
	MaterialsSheet const*	m_spriteSheet = nullptr;
	IntVec2					m_spriteCoords = IntVec2::ZERO;
	Vec2					m_uvAtMins = Vec2::ZERO;
	Vec2					m_uvAtMaxs = Vec2::ZERO;
};
