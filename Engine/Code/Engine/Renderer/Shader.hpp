#pragma once
#include "Engine/Renderer/RenderBuffer.hpp"
#include <string>

//constexpr int NUM_ELEMENTS = 6;

class RenderContext;
struct ID3D11Resource;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11RasterizerState;
struct ID3D11InputLayout;
struct ID3D10Blob;


enum eShaderType
{
	SHADER_TYPE_VERTEX,
	SHADER_TYPE_FRAGMENT,
};

class ShaderStage
{
public:
	~ShaderStage();

	bool Compile( RenderContext* ctx,
		std::string const& filename,
		void const* source, // East const style
		size_t const sourceByteLen,
		eShaderType stage );

	bool IsValid() const { return ( nullptr != m_handle); }

	void const* GetByteCode() const;
	size_t GetByteCodeLength() const;


public:
	eShaderType m_type;
	ID3D10Blob* m_byteCode = nullptr; 
	union 
	{
			ID3D11Resource *m_handle;
			ID3D11VertexShader *m_vs;
			ID3D11PixelShader *m_fs;
	};
};

class Shader
{
public:
	Shader( RenderContext* context );
	~Shader();
	bool CreateFromFile( std::string const& filename );
	void CreateRasterState();

	std::string GetFilePath() { return m_filePath; }
	

	//ID3D11InputLayout* GetOrCreateInputLayout();
	ID3D11InputLayout* GetOrCreateInputLayout( buffer_attribute_t const* layout );

public:
	RenderContext* m_owner = nullptr; 
	ShaderStage m_vertexStage;
	ShaderStage m_fragmentStage;

	std::string m_filePath;

	//ID3D11RasterizerState* m_rasterState = nullptr;
	ID3D11InputLayout* m_inputLayout;

	//buffer_attribute_t const* m_lastUsedLayout;
};
