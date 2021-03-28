#pragma once
#include <string>

std::string errorShaderString = R"(
struct vs_input_t
{
   // we are not defining our own input data;
   float3 position      : POSITION;
   float4 color         : COLOR;
   float2 uv            : TEXCOORD;
};
static float SHIFT = 0.75f;
cbuffer time_constants : register(b0)
{
	float SYSTEM_TIME_SECONDS;
	float SYSTEM_TIME_DELTA_SECONDS;
};

cbuffer camera_constants : register(b1)
{
	float4x4 PROJECTION;
	float4x4 VIEW;
};

Texture2D <float4> tDiffuse : register(t0);
SamplerState sSampler: register(s0);


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t
{
   float4 position : SV_POSITION;
   float4 color : COLOR;
   float2 uv : UV;
};
float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
	float domain = inMax - inMin;
	float range = outMax - outMin;
	return ((val - inMin) / domain) * range + outMin;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
   v2f_t v2f = (v2f_t)0;
   // forward vertex input onto the next stage
   v2f.position = float4( input.position, 1.0f );
   v2f.color = input.color;
   v2f.uv = input.uv;

   float4 worldPos = float4( input.position, 1 );
   float4 cameraPos = mul( VIEW, worldPos );
   float4 clipPos = mul(PROJECTION, cameraPos); 

   v2f.position = clipPos;

   return v2f;
}
// raster step
// float3
//--------------------------------------------------------------------------------------
// Fragment Shader
//
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	//float4 color = tDiffuse.Sample( sSampler, input.uv );
	//return color * input.color;

	float4 color = float4( 1.0f, 0.f, 1.0f, 1.0f );   
	return color;
}
)"; 

