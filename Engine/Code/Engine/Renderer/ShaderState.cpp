#include "Engine/Renderer/ShaderState.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <map>

extern RenderContext* g_theRenderer;
std::map< std::string, ShaderState*>  ShaderState::s_definitions;

ShaderState::ShaderState( const XmlElement& shaderStateXmlElement )
{
	m_shader = g_theRenderer->GetOrCreateShader( ParseXmlAttribute( shaderStateXmlElement, "Shader", "Data/Shaders/Default.hlsl" ).c_str() );
	m_blendMode = GetBlendModeFromText( ParseXmlAttribute( shaderStateXmlElement, "BlendMode", "Opaque" ) );
	m_depthTest = GetDepthTestModeFromText( ParseXmlAttribute( shaderStateXmlElement, "DepthTest", "LEqual" ) );
	m_writeDepth = ParseXmlAttribute( shaderStateXmlElement, "DepthWrite", true );
	m_isWindingOrderCCW = ParseXmlAttribute( shaderStateXmlElement, "Winding", true );
	m_culling = GetCullModeFromText( ParseXmlAttribute( shaderStateXmlElement, "Culling", "Back" ) );
	m_fillMode = GetFillModeFromText( ParseXmlAttribute( shaderStateXmlElement, "FillMode", "Solid" ) );
}

ShaderState::~ShaderState()
{
}

void ShaderState::SetupFromXML( const std::string& xmlFilePath )
{
	XmlDocument xmlDocument;
	const char* defXmlFilePathPointer = &xmlFilePath[0];
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

	XmlElement* shaderStateDefElement = rootElement->FirstChildElement();
	while( shaderStateDefElement )
	{
		std::string defName = ParseXmlAttribute( *shaderStateDefElement, "name", "Unnamed" );
		ShaderState* shaderStateDef = new ShaderState( *shaderStateDefElement );
		shaderStateDef->m_filePath = xmlFilePath;
		s_definitions[defName] = shaderStateDef;
		shaderStateDefElement = shaderStateDefElement->NextSiblingElement();
	}
}

BlendMode ShaderState::GetBlendModeFromText( const std::string& text )
{
	if( text == "Opaque" || text == "opaque" ) {
		return BlendMode::_OPAQUE;
	}
	if( text == "Alpha" || text == "alpha" ) {
		return BlendMode::ALPHA;
	}
	if( text == "Additive" || text == "additive" ) {
		return BlendMode::ADDITIVE;
	}

	ERROR_AND_DIE( "Failed to load blend mode!" );
}

eCompareOp ShaderState::GetDepthTestModeFromText( const std::string text )
{
	if( text == "Never" || text == "never" ) {
		return eCompareOp::COMPARE_FUNC_NEVER;
	}
	else if( text == "Always" || text == "always" ) {
		return eCompareOp::COMPARE_FUNC_ALWAYS;
	}
	else if( text == "LEqual" || text == "LessEqual" ) {
		return eCompareOp::COMPARE_FUNC_LEQUAL;
	}
	else if( text == "GEqual" || text == "GreaterEqual" ) {
		return eCompareOp::COMPARE_FUNC_GEQUAL;
	}
	else if( text == "Equal"  || text == "equal" ) {
		return eCompareOp::COMPARE_FUNC_EQUAL;
	}
	else if( text == "Greater" || text == "greater" ) {
		return eCompareOp::COMPARE_FUNC_GREATER;
	}
	else if( text == "Less" || text == "less" ) {
		return eCompareOp::COMPARE_FUNC_LESS;
	}
	else if( text == "NotEqual" || text == "not equal" ) {
		return eCompareOp::COMPARE_FUNC_NOT_EQUAL;
	}
	else {
		ERROR_AND_DIE( "Failed to load depth test mode!" );
	}
}

D3D11_CULL_MODE ShaderState::GetCullModeFromText( const std::string& text )
{
	D3D11_CULL_MODE mode = D3D11_CULL_BACK;

	if( text == "Back" || text == "back" ) {
		mode =  D3D11_CULL_MODE::D3D11_CULL_BACK;
	}
	if( text == "Front" || text == "front" ) {
		mode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
	}
	if( text == "None" || text == "none" ) {
		mode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	}

	return mode;
	//ERROR_AND_DIE( "Failed to load cull mode from xml file." );
}

D3D11_FILL_MODE ShaderState::GetFillModeFromText( const std::string& text )
{
	D3D11_FILL_MODE mode = D3D11_FILL_SOLID;

	if( text == "Wireframe" || text == "wireframe" ) {
		mode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	}
	if( text == "Solid" || text == "front" ) {
		mode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	}
	
	return mode;
}
