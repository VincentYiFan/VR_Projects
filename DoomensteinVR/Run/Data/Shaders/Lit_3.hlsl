#include "Common.hlsl"
#include "Dot3.hlsl"

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
    float3 world_normal : WORLD_NORMAL; 
    float3 world_tangent :WORLD_TANGENT;
    float3 world_bitangent : WORLD_BITANGENT;
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

//fragment_output_t FragmentFunction( v2f_t input )
//{
//    float3 normal = normalize(input.world_normal);
//    float3 tangent = normalize(input.world_tangent);
//    float3 bitangent = normalize(input.world_bitangent);
//
//    //float3x3 tbn = GetWorldToSurfaceTransform( normal, tangent );
//    float3x3 tbn = float3x3( tangent, bitangent, normal );
//
//    float4 diff_color = tDiffuse.Sample( sSampler, input.uv );
//    float4 normal_color = tNormal.Sample( sSampler, input.uv );
//    float4 spec_color = tSpecular.Sample( sSampler, input.uv );
//    //float3 emsv_color = tEmissive.Sample( sSampler, input.uv ).xyz;
//
//    float alpha = ( input.color.a * diff_color.a );
//
//    if( alpha <= 0.0f )
//        discard;
//
//    // convert sRGB images to linear space (remove this if 
//    // images are already defined as sRGB)
//    float3 surface_color = ( input.color * diff_color ).xyz;    // multiply our tint with our texture color to get our final color; 
//    surface_color = pow( surface_color.xyz, GAMMA.xxx );
//    //emsv_color = pow( emsv_color.xyz, GAMMA.xxx );
//
//    float3 surface_normal = NormalColorToVector( normal_color.xyz );
//    float3 world_normal = mul( surface_normal, tbn );
//
//    float spec_factor = spec_color.x * SPECULAR_FACTOR;
//    float3 final_color = ComputeLightingAt( input.world_position, world_normal, surface_color, spec_factor );
//    final_color = pow( final_color.xyz, INVERSE_GAMMA.xxx );
//
//    float3 bloom = GetBloom( input.world_position, world_normal, surface_color, spec_factor );
//
//    final_color = ApplyFog( input.world_position, final_color );
//    //return float4( final_color, alpha );
//
//
//    //final_color = pow( final_color.xyz, INVERSE_GAMMA.xxx );
//
//    fragment_output_t output;
//    output.color = float4( final_color.xyz, alpha );
//    //output.color = float4( normal_color.xyz, alpha );
//    output.bloom = float4( bloom, 1 );
//    output.normal = float4( (world_normal + float3(1,1,1)) * .5f, 1 );
//    //output.normal = float4( world_normal, 1 );
//    output.tangent = float4( (tangent + float3(1,1,1)) * .5f, 1 );
//    output.albedo = diff_color;
//
//    return output;
//}

float4 FragmentFunction( v2f_t input ) : SV_Target0
{
    //float burn_value = tDissolvePattern.Sample( sSampler, input.uv ).x;
    //
    //float burn_min = lerp( -BURN_EDGE_WIDTH, 1.0f, BURN_AMOUNT ); 
    //float burn_max = burn_min + BURN_EDGE_WIDTH; 
    //clip( BURN_AMOUNT - burn_value );
    //
    //float burn_mix = smoothstep( burn_min, burn_max, burn_value ); 
    //float3 burn_color = lerp( BURN_START_COLOR, BURN_END_COLOR, burn_mix );

    float3 normal = normalize(input.world_normal);
    float3 tangent = normalize(input.world_tangent);
    float3 bitangent = normalize(input.world_bitangent);

    //float3x3 tbn = GetWorldToSurfaceTransform( normal, tangent );
    float3x3 tbn = float3x3( tangent, bitangent, normal );

    float4 diff_color = tDiffuse.Sample( sSampler, input.uv );
    float4 normal_color = tNormal.Sample( sSampler, input.uv );
    float4 spec_color = tSpecular.Sample( sSampler, input.uv );
    //float3 emsv_color = tEmissive.Sample( sSampler, input.uv ).xyz;

    float alpha = ( input.color.a * diff_color.a );

    // convert sRGB images to linear space (remove this if 
    // images are already defined as sRGB)
    float3 surface_color = ( input.color * diff_color ).xyz;    // multiply our tint with our texture color to get our final color; 
    surface_color = pow( surface_color.xyz, GAMMA.xxx );
    //emsv_color = pow( emsv_color.xyz, GAMMA.xxx );

    float3 surface_normal = NormalColorToVector( normal_color.xyz );
    float3 world_normal = mul( surface_normal, tbn );

    float spec_factor = spec_color.x * SPECULAR_FACTOR;
    float3 final_color = ComputeLightingAt( input.world_position, world_normal, surface_color, spec_factor );
    final_color = pow( final_color.xyz, INVERSE_GAMMA.xxx );

    //final_color = lerp( burn_color, final_color, 1 - burn_mix ); 

    final_color = ApplyFog( input.world_position, final_color );
    return float4( final_color, alpha );
}