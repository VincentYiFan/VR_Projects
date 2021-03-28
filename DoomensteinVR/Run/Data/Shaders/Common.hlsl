//------------------------------------------------------------------------
// CONSTANTS
//------------------------------------------------------------------------
static const uint MAX_LIGHTS = 8; 
static const float3 GAMMA = (float)2.2f;
static const float3 INVERSE_GAMMA = (float)0.4545f; //1.0f/2.2f

//------------------------------------------------------------------------
cbuffer time_constants : register(b0)
{
    float SYSTEM_TIME_SECONDS;
    float SYSTEM_TIME_DELTA_SECONDS;

    float2 timePad00;
};

//------------------------------------------------------------------------
cbuffer camera_constants : register(b1)
{
    float4x4 PROJECTION;
    float4x4 VIEW;
    float4x4 WORLD_TO_CLIP;
    float4x4 CLIP_TO_WORLD;
    float4x4 CAMERA_MODEL;

    float3 CAMERA_POSITION; float cam_pad00; 
    float3 CAMERA_RIGHT;    float cam_pad01; 
    float3 CAMERA_UP;       float cam_pad02; 
    float3 CAMERA_FORWARD;  float cam_pad03; 
};

//------------------------------------------------------------------------
cbuffer transform_constants: register(b2)
{
    float4x4 MATRIX;

    float SPECULAR_FACTOR; // poor-man version of shininess or wetness
    float SPECULAR_POWER;  // poor-man version smoothness
    float2 pad00; 

    //float3 CAMERA_POSITION;
    //float pad01; 
};

//------------------------------------------------------------------------
cbuffer tint_constants: register(b3)
{
    float4 TINT;
};

//------------------------------------------------------------------------
// structs
//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct light_t 
{
    float3 color;               // color of the light
    float intensity;            // how "bright" it is

    float3 world_position;      // position of the light
    float pad00;                // next member aligns to a 16-byte boundary, adding this to be explicit 

    float3 direction;           // direction light is point, default (0,0,1)
    float direction_factor;     // do we consider this a point light (0.0f) or a directional light (1.0f).  Determine how we calcualte the incident light vector, default (0.0f)

    float3 attenuation;         // attentuation for diffuse light, default (0,0,1)
    float dot_inner_angle;      // cone light inner angle (default -1.0f) - angle at which cone lights begin to stop affecting an object

    float3 spec_attenuation;    // attenuation for specular lighting (constant,linear,quadratic), default (0,0,1)
    float dot_outer_angle;      // cone light outer angle (default -1.0f) - angle at which cone lights stop affecting the object completely
}; 

//------------------------------------------------------------------------
cbuffer light_constants : register(b4)
{
    float4 AMBIENT; 
    float4 EMISSIVE;

    light_t LIGHTS[MAX_LIGHTS]; 

    float DIFFUSE_FACTOR; 
    float EMISSIVE_FACTOR;

    // fog
    float FOG_NEAR_DISTANCE; 
    float FOG_FAR_DISTANCE; 

    float3 FOG_NEAR_COLOR;
    float fog_pad00; 
    float3 FOG_FAR_COLOR;
    float fog_pad01;  
};

//cbuffer material_constants : register(b5)
//{
//    float3 FRESNEL_COLOR; 
//    float FRESNEL_POWER; 
//
//    float3 BURN_START_COLOR;
//    float BURN_EDGE_WIDTH;
//
//    float3 BURN_END_COLOR;
//    float BURN_AMOUNT;
//
//    uint STEP_COUNT;
//    float DEPTH;
//    float mat_pad00[2];
//};

//------------------------------------------------------------------------
float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
    float domain = inMax - inMin;
    float range = outMax - outMin;
    return ((val - inMin) / domain) * range + outMin;
};

//------------------------------------------------------------------------
float3 NormalColorToVector( float3 color ) 
{
    return normalize(color * float3( 2.0f, 2.0f, 1.0f ) - float3( 1.0f, 1.0f, 0.0f )); 
};

//------------------------------------------------------------------------
float3x3 GetWorldToSurfaceTransform( float3 normal, float4 tangent )
{
   float3 n = normalize(normal); 
   float3 t = normalize(tangent.xyz); 
   float3 b = normalize( cross(n, t) ) * tangent.w; 
   return float3x3( t, b, n ); 
};

//------------------------------------------------------------------------
float3x3 GetSurfaceToWorldTransform( float3 normal, float4 tangent ) 
{
   return transpose( GetWorldToSurfaceTransform( normal, tangent ) ); 
}

//------------------------------------------------------------------------
float3 ApplyFog( float3 world_pos, float3 color ) 
{
   float dist = length( world_pos - CAMERA_POSITION );
   float t = smoothstep( FOG_NEAR_DISTANCE, FOG_FAR_DISTANCE, dist ); 
   return lerp( color, FOG_FAR_COLOR, t.xxx );  
}
