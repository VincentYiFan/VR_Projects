//--------------------------------------------------------------------------------------
// Stream Input
// ------
// Stream Input is input that is walked by the vertex shader.
// If you say "Draw(3,0)", you are telling to the GPU to expect '3' sets, or
// elements, of input data.  IE, 3 vertices.  Each call of the VertxShader
// we be processing a different element.
//--------------------------------------------------------------------------------------
// inputs are made up of internal names (ie: uv) and semantic names
// (ie: TEXCOORD).  "uv" would be used in the shader file, where
// "TEXCOORD" is used from the client-side (cpp code) to attach ot.
// The semantic and internal names can be whatever you want,
// but know that semantics starting with SV_* usually denote special
// inputs/outputs, so probably best to avoid that naming.

#include "Common.hlsl"
struct vs_input_t
{
    // we are not defining our own input data;
    float3 position      : POSITION;
    float4 color         : COLOR;
    float2 uv            : TEXCOORD;

    float3 tangent       : TANGENT;
    float3 bitangent     : BITANGENT;
    float3 normal        : NORMAL;
};


cbuffer fresnel_constants : register(b5)
{
    float3 FRESNEL_COLOR; 
    float FRESNEL_POWER; 
};


Texture2D <float4> tDiffuse : register(t0);
Texture2D <float4> tNormal : register(t1);
Texture2D <float4> tSpecular : register(t2);
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

    float3 world_position : WORLD_POSITION; 
    float3 world_normal : WORLD_NORMAL; 
    float3 world_tangent :WORLD_TANGENT;
    float3 world_bitangent : WORLD_BITANGENT;

};


//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
    v2f_t v2f = (v2f_t)0;

    // move the vertex through the spaces
    float4 local_pos = float4( input.position, 1.0f );    // passed in position is usually inferred to be "local position", ie, local to the object
    float4 world_pos = mul( MATRIX, local_pos );       // world pos is the object moved to its place int he world by the model, not used yet
    float4 camera_pos = mul( VIEW, world_pos );
    float4 clip_pos = mul( PROJECTION, camera_pos ); 


    // normal is currently in model/local space
    float4 local_normal = float4( input.normal, 0.0f );
    float4 world_normal = mul( MATRIX, local_normal );

    float4 local_tangent = float4( input.tangent, 0.0f );
    float4 world_tangent = mul( MATRIX, local_tangent);

    float4 local_bitangent = float4( input.bitangent, 0.0f );
    float4 world_bitangent = mul( MATRIX, local_bitangent);


    // forward vertex input onto the next stage
    v2f.position = clip_pos;
    v2f.color = input.color * TINT;
    v2f.uv = input.uv;
    v2f.world_position = world_pos.xyz;

    v2f.world_normal = world_normal.xyz;
    v2f.world_tangent = world_tangent.xyz;
    v2f.world_bitangent = world_bitangent.xyz;

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
    float3 normal = normalize(input.world_normal);
    float3 tangent = normalize(input.world_tangent);
    float3 bitangent = normalize(input.world_bitangent);

    float3x3 tbn = float3x3(tangent, bitangent, normal);

    float3 dir_to_surface = normalize( input.world_position - CAMERA_POSITION );

    float4 normal_color = tNormal.Sample( sSampler, input.uv );
    float3 surface_normal = NormalColorToVector( normal_color.xyz );
    float3 world_normal = mul( surface_normal, tbn );

    float dp = length( cross( dir_to_surface, world_normal ) );

    float factor = pow( dp, FRESNEL_POWER * 16.0f + 15.f );

    float4 outColor = float4( FRESNEL_COLOR, factor );

    return outColor;
}