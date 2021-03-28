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

cbuffer transform_constants: register(b2)
{
    float4x4 MATRIX;

    float SPECULAR_FACTOR; // poor-man version of shininess or wetness
    float SPECULAR_POWER;  // poor-man version smoothness
    float2 pad00; 

    float3 CAMERA_POSITION;
    float pad01; 
};

cbuffer tint_constants: register(b3)
{
    float4 TINT;
};

struct light_t
{
    float3 world_position;
    float pad02;      // this is not required, but know the GPU will add this padding to make the next variable 16-byte aligned
    float3 color;
    float intensity;  // rgb and an intensity

    float3 attenuation;
    float pad03;
};

// buffer telling us information about our camera
cbuffer light_constants : register(b4)
{
    float4 AMBIENT; 
    light_t LIGHT; 
};

Texture2D <float4> tDiffuse     : register(t0);
Texture2D <float4> tNormal      : register(t1);
Texture2D <float4> tSpecular    : register(t2);
Texture2D <float4> tXColor      : register(t8);
Texture2D <float4> tXNormal     : register(t9);
Texture2D <float4> tYColor      : register(t10);
Texture2D <float4> tYNormal     : register(t11);
Texture2D <float4> tZColor      : register(t12);
Texture2D <float4> tZNormal     : register(t13);
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
float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
    float domain = inMax - inMin;
    float range = outMax - outMin;
    return ((val - inMin) / domain) * range + outMin;
};

float3 NormalColorToVector( float3 color ) 
{
    return normalize(color * float3( 2.0f, 2.0f, 1.0f ) - float3( 1.0f, 1.0f, 0.0f )); 
}

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
    float4 outColor = (float4)0

    float2 z_uv = frac( input.world_position.xy ); // value - floor(value)
    //z_uv.y = 1.0f - z_uv.y; // only in glsl to invert

    float2 x_uv = frac( input.world_position.zy );
    //x_uv.x = 1.0f - x_uv.x;
    //x_uv.y = 1.0f - x_uv.y;

    float2 y_uv = frac( input.world_position.xz );

    float4 x_color = tDiffuse.Sample( sSampler, x_uv );
    float4 y_color = tDiffuse.Sample( sSampler, y_uv );
    float4 z_color = tDiffuse.Sample( sSampler, z_uv ); 

   //float4 x_nomral = ;
   //float4 y_nomral = ;
   //float4 z_nomral = ;
    // need float3 x_normal, y_normal...

    float3 weights = normalize( input.world_normal );
    weights = abs(weights);
    float sum = weights.x + weights.y + weights.z;
    weights /= sum;
    //weights = weights * weights; // x*x, y*y, z*z ( x^2 + y^2 + z^2 == 1)

    float4 final_color
        = weights.x * x_color
        + weights.y * y_color
        + weights.z * z_color;


  

    float3 final_normal_color
        = weights.x * x_normal
        + weights.y * y_normal
        + weights.z * z_normal;

   

    outColor = final_color;
    return outColor;
    
    //outColor = float4( z_uv, 0, 1 );
    //return outColor;


   
}