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

cbuffer parallax_constants : register(b5)
{
    uint STEP_COUNT;
    float DEPTH;
    float mat_pad00[2];
};



Texture2D <float4> tDiffuse : register(t0);
Texture2D <float4> tNormal : register(t1);
Texture2D <float4> tSpecular : register(t2);
Texture2D <float4> tEmissive : register(t3);
Texture2D <float4> tDissolvePattern : register(t8);
Texture2D <float4> tHeight : register(t9);
SamplerState sSampler: register(s0);

//--------------------------------------------------------------------------------------
// local dir is the vector that is 
float2 ComputeShallowParallaxUV( float3 local_dir, float2 uv_start )
{
    // move a little farther sepending on the height
    // this is a full step (-DEPTH to adjust V is opposite bitangent)
    float2 uv_move = local_dir.xy * float2( DEPTH, DEPTH );
    float2 uv_end = uv_start + uv_move; 

    float h0 = tHeight.Sample( sSampler, uv_start ).x;
    float h1 = tHeight.Sample( sSampler, uv_end ).x;

    float t = (1 - h0) / ((h1 - h0) + 1); // edge case I'm not handling -> (h1 - h0) == 1
    return lerp( uv_start, uv_end, t ); 
}  

//--------------------------------------------------------------------------------------
float2 ComputeDeepParallaxUV( float3 local_dir, float2 uv_start )
{
    float2 uv = uv_start; 

    const float step = 1.0f / float(STEP_COUNT); 
    float2 uv_move = (local_dir / abs(local_dir.z)).xy * step * DEPTH * 0.5f; // split it evenly across z

    float z = 1.0f; 
    float height = 0.0f; 

    uint i = 0; 

    // cache off all the heights up front
    float heights[32]; 
    [unroll]
    for (i = 0; i < STEP_COUNT; ++i) {
        heights[i] = tHeight.Sample( sSampler, uv ).x;
        uv += uv_move; 
    }

    // always start as 1, as we need the value at the end; 
    [unroll]
    for (i =  1; i < STEP_COUNT; ++i) {
        // go until we fall below the height
        float h1 = heights[i]; 
        if ((z - step) <= h1) {
            float2 uv1 = uv_start + i * uv_move; 
            float2 uv0 = uv1 - uv_move;

            float h0 = heights[i - 1]; 
            float dh = h1 - h0; 

            // now, this is two intersecting lines.  
            // return (d0 / ds).xx;
            // sreturn uv1;
            float t = ((h0 - z) / (-step - dh)); 
            return lerp( uv0, uv1, t ); 
            // return (d1 * uv0 + d0 * uv1) / ds; 
        } 
        z -= step;
    }

    // will not actually get here; 
    return uv;
}

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
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
    float3 normal = normalize(input.world_normal);
    float3 tangent = normalize(input.world_tangent);
    float3 bitangent = normalize(input.world_bitangent);
    float3x3 tbn = float3x3( tangent, bitangent, normal );
    //float3x3 w2s = GetWorldToSurfaceTransform( input.world_normal, float4(input.world_tangent, 0.0f) );
    float3 look_dir = normalize(input.world_position - CAMERA_POSITION); 
    //float3 surf_look_dir = mul( look_dir, tbn );
    float3 surf_look_dir = mul( tbn, look_dir );

    float2 uv = ComputeDeepParallaxUV( surf_look_dir, input.uv ); 
    //float2 uv = ComputeShallowParallaxUV( surf_look_dir, input.uv ); 

    float4 diff_color = tDiffuse.Sample( sSampler, uv );
    float4 normal_color = tNormal.Sample( sSampler, uv );
    float4 spec_color = tSpecular.Sample( sSampler, uv );
    //float3 emsv_color = tEmissive.Sample( sSampler, input.uv ).xyz;

    float alpha = ( input.color.a * diff_color.a );

    // convert sRGB images to linear space (remove this if 
    // images are already defined as sRGB)
    float3 surface_color = ( input.color * diff_color ).xyz;    // multiply our tint with our texture color to get our final color; 
    surface_color = pow( surface_color.xyz, GAMMA.xxx );
    //emsv_color = pow( emsv_color.xyz, GAMMA.xxx );

    float3 surface_normal = NormalColorToVector( normal_color.xyz );
    float3 world_normal = mul( surface_normal, tbn );
    //float3 world_normal = mul( w2s, surface_normal );

    float spec_factor = spec_color.x * SPECULAR_FACTOR;
    float3 final_color = ComputeLightingAt( input.world_position, world_normal, surface_color, spec_factor );

    final_color = pow( final_color.xyz, INVERSE_GAMMA.xxx );
    final_color = ApplyFog( input.world_position, final_color );

    return float4( final_color, alpha );
}