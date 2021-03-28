#include "Common.hlsl"

// input to the vertex shader - for now, a special input that is the index of the vertex we're drawing
struct vs_input_t
{
    uint vidx : SV_VERTEXID; // SV_* stands for System Variable (ie, built-in by D3D11 and has special meaning)
                             // in this case, is the the index of the vertex coming in.
};

cbuffer image_effect_constants : register(b5)
{
    float GREYSCALE;
    float3 imageEffectPad00;
};

// Textures & Samplers are also a form of constant
// data - unform/constant across the entire call
// tSlots are for big things
Texture2D <float4> tDiffuse   : register(t0);   // color of the surface
SamplerState sSampler : register(s0);     // sampler are rules on how to sample color per pixel



                                          
struct VertexToFragment_t
{
    // SV_POSITION is a semantic - or a name identifying this variable. 
    // Usually a semantic can be any name we want - but the SV_* family
    // usually denote special meaning (SV = System Variable I believe)
    // SV_POSITION denotes that this is output in clip space, and will be 
    // use for rasterization.  When it is the input (pixel shader stage), it will
    // actually hold the pixel coordinates.
    float4 position : SV_POSITION;
    float2 uv : UV; 
};

//--------------------------------------------------------------------------------------
// constants
//--------------------------------------------------------------------------------------
// The term 'static' refers to this an built into the shader, and not coming
// from a contsant buffer - which we'll get into later (if you remove static, you'll notice
// this stops working). 
static float3 POSITIONS[3] = {
    float3(-1.0f, -1.0f, 0.0f), 
    float3( 3.0f, -1.0f, 0.0f), 
    float3(-1.0f,  3.0f, 0.0f)
};

static float2 UVS[3] = {
    float2( 0.0f,  1.0f ), 
    float2( 2.0f,  1.0f ), 
    float2( 0.0f,  -1.0f )
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VertexToFragment_t VertexFunction( vs_input_t input )
{
    VertexToFragment_t v2f = (VertexToFragment_t)0;

    // The output of a vertex shader is in clip-space, which is a 4D vector
    // so we need to convert out input to a 4D vector.
    v2f.position = float4( POSITIONS[input.vidx], 1.0f );
    v2f.uv = UVS[input.vidx]; 

    // And return - this will pass it on to the next stage in the pipeline;
    return v2f;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
// If I'm only returning one value, I can optionally just mark the return value with
// a SEMANTIC - in this case, SV_TARGET0, which means it is outputting to the first colour 
// target.
float4 FragmentFunction( VertexToFragment_t input ) : SV_Target0 // semeantic of what I'm returning
{
    float4 color = tDiffuse.Sample( sSampler, input.uv );
    //return color;

    // invert color 
    //return float4( float3(1,1,1) - color.xyz, 1 );

    
    // grayscale
    float3 albedo = dot(color.xyz, float3(0.3, 0.59, 0.11));
   /* color.x = lerp( albedo.x, color.x, 1 );
    color.y = lerp( albedo.y, color.y, 1 );
    color.z = lerp( albedo.z, color.z, 1 );*/
    float3 final_color = lerp( color.xyz, albedo.xyz, GREYSCALE );
    return float4( final_color, color.a );
}