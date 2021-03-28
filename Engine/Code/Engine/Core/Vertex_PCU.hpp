#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include <cstddef>

typedef unsigned int uint;

enum eBufferFormatType // Move to render common later
{
	BUFFER_FORMAT_VEC2,					// DXGI_FORMAT_R32G32-FLOAT
	BUFFER_FORMAT_VEC3,					// DXGI_FORMAT_R32G32B32_FLOAT
	BUFFER_FORMAT_R8G8B8A8_UNORM,

	BUFFER_FORMAT_NULL,
};

struct buffer_attribute_t
{
	std::string name; 			// used to link to a D3D11 shader
	// uint location; 			// used to link to a GL/Vulkan shader 
	eBufferFormatType type = BUFFER_FORMAT_NULL;		// what data are we describing
	uint offset; 				// where is it relative to the start of a vertex

	buffer_attribute_t( char const* n, eBufferFormatType t, uint offset );
	buffer_attribute_t();
};


struct Vertex_PCU 
{
public:
	 Vec3 m_position;
	 Rgba8 m_color;
	 Vec2 m_uvTexCoords;

	 static const buffer_attribute_t LAYOUT[]; 

	 Vertex_PCU() = default;  // default constructor added
	 explicit Vertex_PCU( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords );
	 explicit Vertex_PCU( const Vec2& position, const Rgba8& tint, const Vec2& uvTexCoords );
};
