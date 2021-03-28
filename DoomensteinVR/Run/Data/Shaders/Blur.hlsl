#include "Common.hlsl"

// input to the vertex shader - for now, a special input that is the index of the vertex we're drawing
struct vs_input_t
{
    uint vidx : SV_VERTEXID; // SV_* stands for System Variable (ie, built-in by D3D11 and has special meaning)
                             // in this case, is the the index of the vertex coming in.
    float3 position      : POSITION;
    float4 color         : COLOR;
    float2 uv            : TEXCOORD;
};

//static  const float rt_w = 1728.0;
//static const float rt_h = 864;

//cbuffer dissolve_constants : register(b5)
//{
//    float3 SIZE;
//};

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
    float4 color : COLOR;
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

    v2f.color = input.color;
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
    uint rt_w;
    uint rt_h;
    tDiffuse.GetDimensions( rt_w, rt_h );

    float uXPixelDistance = 1.0/rt_w;
    float uYPixelDistance = 1.0/rt_h;

    float jump = 4.0;
    float pointRange = 10.0;

    float4 color = float4( 0, 0, 0, 0 );
    float2 p = float2( 0, 0 );
    int count = 0;

    for(float u = -pointRange; u < pointRange ; u+=jump) {
        for(float v = -pointRange ; v < pointRange ; v+=jump) {
            p.x = input.uv.x  + u * uXPixelDistance;
            p.y = input.uv.y  + v * uYPixelDistance;

            // If the point is within the range[0, 1]
            if (p.y >= 0.0f && p.x >= 0.0f &&
                p.y <= 1.0f && p.x <= 1.0f ) {
                ++count;
                color += tDiffuse.Sample( sSampler, p );
            }
        }
    }

    color.x /= count;
    color.y /= count;
    color.z /= count;

    //color = color / float( count );
    return float4( color.xyz, 1.0 );
}