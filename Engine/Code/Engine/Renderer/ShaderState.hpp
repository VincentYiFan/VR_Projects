#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include <d3d11.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
class Shader;
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

class ShaderState
{
public:
	explicit ShaderState( const XmlElement& shaderStateXmlElement );
	~ShaderState();

public:
	static void SetupFromXML( const std::string& xmlFilePath );

private:
	BlendMode GetBlendModeFromText( const std::string& text );
	eCompareOp GetDepthTestModeFromText( const std::string text );
	D3D11_CULL_MODE GetCullModeFromText ( const std::string& text );
	D3D11_FILL_MODE GetFillModeFromText( const std::string& text );

public:
	Shader* m_shader = nullptr;
	BlendMode m_blendMode		= BlendMode::_OPAQUE;
	eCompareOp m_depthTest		= eCompareOp::COMPARE_FUNC_LEQUAL;
	bool m_writeDepth			= true;
	bool m_isWindingOrderCCW	= true;
	D3D11_CULL_MODE m_culling	= D3D11_CULL_BACK;
	D3D11_FILL_MODE m_fillMode	= D3D11_FILL_SOLID;

	std::string m_filePath;

	static std::map< std::string, ShaderState*> s_definitions;
};