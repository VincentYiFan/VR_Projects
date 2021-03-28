struct light_factors 
{
    float3 diffuse;
    float3 specular;
};

//------------------------------------------------------------------------
light_factors ApplyLight( light_t light, float3 surface_color, float3 normal, float3 surface_position, float3 dir_to_eye )
{
    float3 dir_to_light = (float3)0;
    float attenuation = 1.0f;

    if( light.direction_factor == 1.0f ) {
        // direction light
        dir_to_eye = normalize( light.world_position );
        attenuation = 1.0f;
    }
    else {
        // point light
        dir_to_light = normalize( light.world_position - surface_position );
        float distance_to_light = length( light.world_position - surface_position );
        attenuation = 1.0f / ( (light.attenuation.x) + (light.attenuation.y * distance_to_light) + (light.attenuation.z * distance_to_light * distance_to_light) );
    
    	// cone restrictions (affects attenuation)
    	float lightToSurfaceAngle = degrees(acos(dot(-dir_to_light, normalize(light.direction))));
    	if(lightToSurfaceAngle > light.dot_outer_angle){
    		attenuation = 0.0f;
    	}
    }

    // ambient
    //float3 ambient = AMBIENT.xyz * AMBIENT.w * surface_color;

    // diffuse
    float diffuseCoefficient = max( 0.0f, dot( normal, dir_to_light ) );
    float3 diffuse = diffuseCoefficient /** surface_color*/ * light.color * light.intensity; // A07: added light intensity here

    // specular
    float specularCoefficient = 0.0f;
    if( diffuseCoefficient > 0.0f )
        specularCoefficient = pow( max( 0.0f, dot( dir_to_eye, reflect( -dir_to_light, normal ) ) ), SPECULAR_POWER );
    float3 specular = specularCoefficient * light.color * SPECULAR_FACTOR;

    light_factors lightFactors;
    lightFactors.diffuse = diffuse * attenuation;
    lightFactors.specular = specular * attenuation;
    return lightFactors;

    // linear light
    //return attenuation * ( diffuse + specular );
};

//--------------------------------------------------------------------------------------
float2 ComputeLightFactor( light_t light, float3 world_pos, float3 world_normal, float3 dir_to_eye )
{
   float3 vec_to_light = light.world_position - world_pos; 

   float dist = length(vec_to_light);
   float plane_dist = abs( dot( world_pos - light.world_position, light.direction ) ); 
   dist = lerp( dist, plane_dist, light.direction_factor ) + 0.00048828125f; //  little nudge to prevenet 0 
   
   float3 dir_to_light = normalize(vec_to_light);
   
   float3 light_dir = lerp( -dir_to_light, light.direction, light.direction_factor ); 
   float3 att_vec = float3( 1.0f, dist, dist * dist ); 
   
   // how much do these two coincide (cone lights)
   float dot_angle = dot( -dir_to_light, light.direction ); 
   float cone_att = smoothstep( light.dot_outer_angle, light.dot_inner_angle, dot_angle ); 

   float diff_att = light.intensity / dot( att_vec, light.attenuation ) * cone_att;  
   float spec_att = light.intensity / dot( att_vec, light.attenuation ) * cone_att; 

   // compute diffuse
   // max prevents lighting from the "back", which would subtract light
   float dot_incident = dot( -light_dir, world_normal ); 
   float facing = smoothstep( -0.4f, 0.1f, dot_incident); 
   float diffuse = max( 0.0f, dot_incident ); 
   
   // phong
   float3 ref_dir_to_light = reflect( light_dir, world_normal );  
   float specular = max( 0.0f, dot( ref_dir_to_light, dir_to_eye ) );

   // finalize specular
   specular = facing * specular; 
   specular = pow( specular, SPECULAR_POWER ); 

   // return result
   return float2( diff_att * diffuse, spec_att * specular ); 
};
//--------------------------------------------------------------------------------------

//------------------------------------------------------------------------
// be sure colors are in linear space when passed in; 
float3 ComputeLightingAt( float3 world_pos, float3 world_normal, 
   float3 surf_color, float spec_factor )
{
    float3 dir_to_eye = normalize(CAMERA_POSITION - world_pos); 

   float3 diffuse = AMBIENT.xyz * AMBIENT.w; // assumes ambient is set from a user - so sRGB space
   float3 spec = float3(0.0f, 0.f, 0.f); 

   // add up contribution of all lights
   for (uint i = 0; i < MAX_LIGHTS; ++i)
   {
      float3 light_color = LIGHTS[i].color.xyz;
      float2 light_factors = ComputeLightFactor( LIGHTS[i], world_pos, world_normal, dir_to_eye );
      
      diffuse += light_factors.x * light_color; 
      spec += light_factors.y * light_color; 
   }

   // limit it
   diffuse = min( diffuse, float3(1,1,1) );
   diffuse = saturate( diffuse );

   spec *= spec_factor;  // scale back specular based on spec factor

   // returns light color (in linear space)
   //return diffuse * surf_color + spec;   // TEMP
   return diffuse * surf_color /*+ spec*/; 
}

float3 GetBloom( float3 world_pos, float3 world_normal, 
    float3 surf_color, float spec_factor )
{
    float3 dir_to_eye = normalize(CAMERA_POSITION - world_pos); 

    float3 diffuse = AMBIENT.xyz * AMBIENT.w; // assumes ambient is set from a user - so sRGB space
    float3 spec = float3(0.0f, 0.f, 0.f); 

    // add up contribution of all lights
    for (uint i = 0; i < MAX_LIGHTS; ++i)
    {
        float3 light_color = LIGHTS[i].color.xyz;
        float2 light_factors = ComputeLightFactor( LIGHTS[i], world_pos, world_normal, dir_to_eye );

        diffuse += light_factors.x * light_color; 
        spec += light_factors.y * light_color; 
    }

    diffuse = min( diffuse, float3(1,1,1) );
    float3 bloom = max( float3(0,0,0), spec - float3(1,1,1) );
    diffuse = saturate( diffuse );

    //spec *= spec_factor;  // scale back specular based on spec factor

                          // returns light color (in linear space)
    return bloom; 
}
