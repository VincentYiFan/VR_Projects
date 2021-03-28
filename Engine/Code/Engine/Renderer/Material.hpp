#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include <vector>
#include <map>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
class Shader;
class ShaderState;
class Sampler;
class Texture;
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

class Material
{
public:
	explicit Material( RenderContext* ctx,  const char* shaderStateName );
	explicit Material( RenderContext* ctx, const XmlElement& matDefXmlElement );
	~Material();

	static void LoadDefinitions( RenderContext* ctx, const std::string& definitionXmlFilePath );
	static std::map<std::string, Material*> s_definitions;

	void SetData( void const* data, size_t data_size );

	template <typename UBO_STRUCT_TYPE>
	void SetData( UBO_STRUCT_TYPE const& data )
	{
		SetData( &data, sizeof( UBO_STRUCT_TYPE ) );
	}

	template <typename UBO_STRUCT_TYPE>
	UBO_STRUCT_TYPE* GetDataAs()
	{
		m_uboIsDirty = true;
		if( m_uboCPUData.size() == sizeof( UBO_STRUCT_TYPE ) ) {
			return (UBO_STRUCT_TYPE*)&m_uboCPUData[0];
		}
		else {
			m_uboCPUData.resize( sizeof( UBO_STRUCT_TYPE ) );
			UBO_STRUCT_TYPE* retPtr = (UBO_STRUCT_TYPE*)&m_uboCPUData[0];
			new (retPtr) UBO_STRUCT_TYPE();

			return retPtr;
		}
	}

	void SetShaderState( ShaderState* shaderState );
	void SetDiffuseTexture( Texture* texture );	
	void SetNormalTexture( Texture* texture );
	void SetSampler( Sampler* sampler );
	void UpdateUBOIfDirty();

public:
	RenderContext* m_owner = nullptr;
	ShaderState* m_shaderState = nullptr;

	// renderer specific
	Rgba8 m_tint = Rgba8::WHITE;
	float m_specularFactor = 1.f;
	float m_specularPower = 32.f;

	// general data (and maybe diffuse)
	//std::vector<Texture*> m_texturesPerSlot;
	//std::vector<Sampler*> m_samplersPerSlot;

	std::map<uint, Texture*> m_textureIdxPointer;
	std::map<uint, Sampler*> m_samplerIdxPointer;

	std::vector<unsigned char> m_uboCPUData;
	RenderBuffer* m_ubo = nullptr;
	bool m_uboIsDirty = false;
};