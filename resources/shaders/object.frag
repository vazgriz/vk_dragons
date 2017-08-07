#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input: tangent space matrix, position (view space) and uv coming from the vertex shader
layout(location = 0) in mat3 Intbn;
layout(location = 3) in vec3 Inposition; 
layout(location = 4) in vec2 Inuv;
layout(location = 5) in vec3 InlightSpacePosition;
layout(location = 6) in vec3 InmodelPosition;

// Uniform: the light structure (position in view space)
layout(set = 0, binding = 0) uniform Uniforms {
    mat4 camProjection;
    mat4 camView;
    mat4 rotationOnlyView;
    mat4 camViewInverse;
    mat4 lightProjection;
    mat4 lightView;
	vec4 lightPosition;
	vec4 lightIa;
	vec4 lightId;
	vec4 lightIs;
	float lightShininess;
} uniforms;


layout(set = 1, binding = 0) uniform sampler2D textureColor;
layout(set = 1, binding = 1) uniform sampler2D textureNormal;
layout(set = 1, binding = 2) uniform sampler2D textureEffects;
layout(set = 1, binding = 3) uniform samplerCube textureCubeMap;
layout(set = 1, binding = 4) uniform samplerCube textureCubeMapSmall;
layout(set = 1, binding = 5) uniform sampler2D shadowMap;

// Output: the fragment color
layout(location = 0) out vec4 fragColor;

// Returns a random float in [0,1] based on the input vec4 seed.
float random(vec4 p){
	return fract(sin(dot(p, vec4(12.9898,78.233,45.164,94.673))) * 43758.5453);
}


void main(){
	// Compute the normal at the fragment using the tangent space matrix and the normal read in the normal map.
	vec3 n = texture(textureNormal,Inuv).rgb;
	n = normalize(n * 2.0 - 1.0);
	n = normalize(Intbn * n);

	// Read the effects values
	vec3 effects = texture(textureEffects,Inuv).rgb;

	// Compute the direction from the point to the light
	// light.position.w == 0 if the light is directional, 1 else.
	vec3 d = normalize(uniforms.lightPosition.xyz - uniforms.lightPosition.w * Inposition);

	vec3 diffuseColor = texture(textureColor, Inuv).rgb;
	
	vec3 worldNormal = vec3(uniforms.camViewInverse * vec4(n,0.0));
	vec3 lightColor = texture(textureCubeMapSmall,normalize(worldNormal)).rgb;
	diffuseColor = mix(diffuseColor, diffuseColor * lightColor, 0.5);
	
	// The ambient factor
	vec3 ambient = effects.r * 0.3 * diffuseColor;
	
	// Compute the diffuse factor
	float diffuse = max(0.0, dot(d,n));

	vec3 v = normalize(-Inposition);

	// Compute the specular factor
	float specular = 0.0;
	if(diffuse > 0.0){
		vec3 r = reflect(-d,n);
		specular = pow(max(dot(r,v),0.0), uniforms.lightShininess);
		specular *= effects.g;
	}
	
	vec3 reflectionColor = vec3(0.0);
	if(effects.b > 0.0){
		vec3 rCubeMap = reflect(-v, n);
		rCubeMap = vec3(uniforms.camViewInverse * vec4(rCubeMap,0.0));
		reflectionColor = texture(textureCubeMap,rCubeMap).rgb;
	}

	vec3 lightShading = diffuse * diffuseColor + specular * uniforms.lightIs.rgb;
	
	float shadow = 1.0;
	if (InlightSpacePosition.z < 1.0){
		// Read first and second moment from shadow map.
		vec2 moments = texture(shadowMap, InlightSpacePosition.xy).rg;
		// Initial probability of light.
		float probability = float(InlightSpacePosition.z <= moments.x);
		// Compute variance.
		float variance = moments.y - (moments.x * moments.x);
		variance = max(variance, 0.0001);
		// Delta of depth.
		float delta = InlightSpacePosition.z - moments.x;
		// Use Chebyshev to estimate bound on probability.
		float probabilityMax = variance / (variance + delta*delta);
		shadow = max(probability, probabilityMax);
		// Limit light bleeding by rescaling and clamping the probability factor.
		shadow = clamp( (shadow - 0.1) / (1.0 - 0.1), 0.0, 1.0);
	}
	
	// Mix the ambient color (always present) with the light contribution, weighted by the shadow factor.
	vec3 fColor = ambient * uniforms.lightIa.rgb + shadow * lightShading;
	// Mix with the reflexion color.
	fragColor = vec4(mix(fColor,reflectionColor,0.5*effects.b), 0.0);
	
}
