#include "Common.hlsl"
#include "Dot3.hlsl"

struct vs_input_t
{
    // we are not defining our own input data;
    float3 position      : POSITION;
    float4 color         : COLOR;
    float2 uv            : TEXCOORD;

 /*   float3 tangent       : TANGENT;
    float3 bitangent     : BITANGENT;
    float3 normal        : NORMAL;*/
};


Texture2D <float4> tDiffuse : register(t0);
Texture2D <float4> tNormal : register(t1);
Texture2D <float4> tSpecular : register(t2);
Texture2D <float4> tEmissive : register(t3);
Texture2D <float4> tDissolvePattern : register(t8);
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
   /* float3 world_normal : WORLD_NORMAL; 
    float3 world_tangent :WORLD_TANGENT;
    float3 world_bitangent : WORLD_BITANGENT;*/

};

struct fragment_output_t
{
    float4 color    : SV_Target0;
    float4 bloom    : SV_Target1;
    float4 normal   : SV_Target2;
    float4 albedo   : SV_Target3;
    float4 tangent  : SV_Target4;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
    v2f_t v2f = (v2f_t)0;

    // move the vertex through the spaces
    float4 local_pos = float4( input.position, 1.0f );    // passed in position is usually inferred to be "local position", ie, local to the object
    float4 world_pos = mul( MATRIX, local_pos );          // world pos is the object moved to its place int he world by the model, not used yet
    float4 camera_pos = mul( VIEW, world_pos );
    float4 clip_pos = mul( PROJECTION, camera_pos ); 
    // float4 ndc_pos = clip_pos/clip_pos.w  // for sky box


    // normal is currently in model/local space
 /*   float4 local_normal = float4( input.normal, 0.0f );
    float4 world_normal = mul( MATRIX, local_normal );

    float4 local_tangent = float4( input.tangent, 0.0f );
    float4 world_tangent = mul( MATRIX, local_tangent);

    float4 local_bitangent = float4( input.bitangent, 0.0f );
    float4 world_bitangent = mul( MATRIX, local_bitangent);*/


    // forward vertex input onto the next stage
    v2f.position = clip_pos;
    v2f.color = input.color * TINT;
    v2f.uv = input.uv;
    v2f.world_position = world_pos.xyz;

    //v2f.world_normal = world_normal.xyz;
    //v2f.world_tangent = world_tangent.xyz;
    //v2f.world_bitangent = world_bitangent.xyz;

    return v2f;
}
// raster step
// float3
//--------------------------------------------------------------------------------------
// Fragment Shader
//
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
/*fragment_output_t*/float4 FragmentFunction( v2f_t input ): SV_Target0
{
    float4 diff_color = tDiffuse.Sample( sSampler, input.uv );
    float4 shaded_texel = diff_color * input.color;

    float alpha = ( input.color.a * diff_color.a );

    if( alpha <= 0.0f )
        discard;

    if( shaded_texel.x * shaded_texel.y * shaded_texel.z * shaded_texel.a > 0.9f )
    {
      float voob = 0.5 + 0.5 * sin( 2.0 * SYSTEM_TIME_SECONDS - 33.0 * input.uv.x + 33.0 * input.uv.y );
      voob = 1.0 - 0.5 * ( voob * voob * voob * voob * voob * voob * voob * voob );
      shaded_texel.x *= voob;
      shaded_texel.y *= voob;
    }

    return shaded_texel;

    //float3 surface_color = ( input.color * diff_color ).xyz;    // multiply our tint with our texture color to get our final color; 

    //return float4( surface_color, alpha );
}