#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ShaderState.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Texture.hpp"

std::map< std::string, Material*>  Material::s_definitions;

Material::Material( RenderContext* ctx,  const char* shaderStateName )
{
	m_owner = ctx;
	m_shaderState = ctx->GetShaderState( shaderStateName );

	//m_ubo = ctx->m_materialUBO;
	m_ubo = new RenderBuffer( ctx, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
}

Material::Material( RenderContext* ctx, const XmlElement& matDefXmlElement )
{
	// temp
	//m_ubo = ctx->m_materialUBO;
	m_owner = ctx;
	m_ubo = new RenderBuffer( ctx, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );

	const XmlElement* shaderStateElement = matDefXmlElement.FirstChildElement( "ShaderState" );
	if( shaderStateElement ) {
		const std::string shaderStateName = ParseXmlAttribute( *shaderStateElement, "name", "Unnamed" );
		m_shaderState = ShaderState::s_definitions[shaderStateName];
	}

	const XmlElement* albedoElement = matDefXmlElement.FirstChildElement( "Albedo" );
	if( albedoElement ) {
		uint slot = ParseXmlAttribute( *albedoElement, "slot", (uint)0 );

		const std::string albedoTexFilePath = ParseXmlAttribute( *albedoElement, "path", "Data/Textures/White.png" );
		Texture* tex = ctx->CreateOrGetTextureFromFile( albedoTexFilePath.c_str() );
		m_textureIdxPointer[slot] = tex;

		m_tint = ParseXmlAttribute( *albedoElement, "tint", Rgba8::WHITE );
	}

	const XmlElement* normalElement = matDefXmlElement.FirstChildElement( "Normal" );
	if( normalElement ) {
		uint slot = ParseXmlAttribute( *normalElement, "slot", (uint)1 );

		const std::string albedoTexFilePath = ParseXmlAttribute( *normalElement, "path", "Data/Textures/normal_flat.png" );
		Texture* tex = ctx->CreateOrGetTextureFromFile( albedoTexFilePath.c_str() );

		m_textureIdxPointer[slot] = tex;
	}

	const XmlElement* specularElement = matDefXmlElement.FirstChildElement( "Specular" );
	if( specularElement ) {
		uint slot = ParseXmlAttribute( *specularElement, "slot", (uint)2 );

		const std::string albedoTexFilePath = ParseXmlAttribute( *specularElement, "path", "" );
		if( albedoTexFilePath != "" ) {
			Texture* tex = ctx->CreateOrGetTextureFromFile( albedoTexFilePath.c_str() );
			m_textureIdxPointer[slot] = tex;
		}

		m_specularFactor = ParseXmlAttribute( *specularElement, "factor", 1.f );
		m_specularPower = ParseXmlAttribute( *specularElement, "power", 32.f );
	}
	
	const XmlElement* textureElement = matDefXmlElement.FirstChildElement( "Texture" );
	while( textureElement ) {
		uint slot = ParseXmlAttribute( *textureElement, "slot", (uint)8 );

		const std::string texturePath = ParseXmlAttribute( *textureElement, "path", "Data/Textures/White.png" );
		Texture* tex = ctx->CreateOrGetTextureFromFile( texturePath.c_str() );
		m_textureIdxPointer[slot] = tex;

		textureElement = textureElement->NextSiblingElement();
	}
}

Material::~Material()
{
	m_owner = nullptr;

	delete m_ubo;
	m_ubo = nullptr;

	m_shaderState = nullptr;
}

void Material::LoadDefinitions( RenderContext* ctx, const std::string& definitionXmlFilePath )
{
	XmlDocument xmlDocument;
	const char* defXmlFilePathPointer = &definitionXmlFilePath[0];
	xmlDocument.LoadFile( defXmlFilePathPointer );
	if( xmlDocument.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		// print error message
	}

	XmlElement* rootElement = xmlDocument.RootElement();
	if( !rootElement )
	{
		return;
	}

	XmlElement* matDefElement = rootElement->FirstChildElement();
	while( matDefElement )
	{
		std::string defName = ParseXmlAttribute( *matDefElement, "name", "Unnamed" );
		Material* matDef = new Material( ctx, *matDefElement );
		s_definitions[defName] = matDef;
		matDefElement = matDefElement->NextSiblingElement();
	}
}

void Material::SetData( void const* data, size_t dataSize )
{
	m_uboCPUData.resize( dataSize );
	memcpy( &m_uboCPUData[0], data, dataSize );
	m_uboIsDirty = true;
}

void Material::SetShaderState( ShaderState* shaderState )
{
	if( m_shaderState != shaderState ) {
		m_shaderState = shaderState;
	}
}

void Material::SetDiffuseTexture( Texture* texture )
{
	m_textureIdxPointer[0] = texture;
	//m_texturesPerSlot.push_back( texture );
}

void Material::SetNormalTexture( Texture* texture )
{
	m_textureIdxPointer[1] = texture;
	//m_texturesPerSlot.push_back( texture );
}

void Material::SetSampler( Sampler* sampler )
{
	m_samplerIdxPointer[0] = sampler;
}

void Material::UpdateUBOIfDirty()
{
	if( m_uboIsDirty ) {
		size_t dataSize = (size_t)m_uboCPUData.size();
		m_ubo->Update( &m_uboCPUData[0], dataSize, dataSize );
		m_uboIsDirty = false;
	}
}
