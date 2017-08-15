#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input: tangent space matrix, position (view space) and uv coming from the vertex shader
layout(location = 0) in mat3 Intbn;
layout(location = 3) in vec3 Inposition;
layout(location = 4) in vec2 Inuv;
layout(location = 5) in vec3 InlightSpacePosition;
layout(location = 6) in vec3 InmodelPosition;
layout(location = 7) in vec3 IntangentSpacePosition;
layout(location = 8) in vec3 IntangentSpaceView;
layout(location = 9) in vec3 IntangentSpaceLight;

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

#define PARALLAX_MIN 8
#define PARALLAX_MAX 32
#define PARALLAX_SCALE 0.04

// Returns a random float in [0,1] based on the input vec4 seed.
float random(vec4 p){
	return fract(sin(dot(p, vec4(12.9898,78.233,45.164,94.673))) * 43758.5453);
}

// Compute the light shading.

vec3 shading(vec2 uv, vec3 lightPosition, float lightShininess, vec3 lightColor, out vec3 ambient){
	// Compute the normal at the fragment using the tangent space matrix and the normal read in the normal map.
	vec3 n = texture(textureNormal,uv).rgb;
	n = normalize(n * 2.0 - 1.0);
	n = normalize(Intbn * n);
	
	// Compute the direction from the point to the light
	// light.position.w == 0 if the light is directional, 1 else.
	vec3 d = normalize(lightPosition - uniforms.lightPosition.w * Inposition);
	
	vec3 diffuseColor = texture(textureColor, uv).rgb;
	
	vec3 worldNormal = vec3(uniforms.camViewInverse * vec4(n,0.0));
	vec3 ambientLightColor = texture(textureCubeMapSmall,normalize(worldNormal)).rgb;
	diffuseColor = mix(diffuseColor, diffuseColor * ambientLightColor, 0.5);
	
	// The ambient factor
	ambient = 0.3 * diffuseColor;
	
	// Compute the diffuse factor
	float diffuse = max(0.0, dot(d,n));
	
	vec3 v = normalize(-Inposition);
	
	// Compute the specular factor
	float specular = 0.0;
	if(diffuse > 0.0){
		vec3 r = reflect(-d,n);
		specular = pow(max(dot(r,v),0.0),lightShininess);
	}
	
	return diffuse * diffuseColor + specular * lightColor;
}


// Compute the shadow multiplicator based on shadow map.

float shadow(vec3 lightSpacePosition){
	float probabilityMax = 1.0;
	if (lightSpacePosition.z < 1.0
		&& lightSpacePosition.x >= 0.0 && lightSpacePosition.x <= 1.0
		&& lightSpacePosition.y >= 0.0 && lightSpacePosition.y <= 1.0){
		
		// Read first and second moment from shadow map.
		vec2 moments = texture(shadowMap, lightSpacePosition.xy).rg;
		
		// Initial probability of light.
		float probability = float(lightSpacePosition.z <= moments.x);
		// Compute variance.
		float variance = moments.y - (moments.x * moments.x);
		variance = max(variance, 0.00001);
		// Delta of depth.
		float d = lightSpacePosition.z - moments.x;
		// Use Chebyshev to estimate bound on probability.
		probabilityMax = variance / (variance + d*d);
		probabilityMax = max(probability, probabilityMax);
		// Limit light bleeding by rescaling and clamping the probability factor.
		probabilityMax = clamp( (probabilityMax - 0.1) / (1.0 - 0.1), 0.0, 1.0);
	}
	return probabilityMax;
}

// Compute the new UV coordinates for the parallax mapping effect.

vec2 parallax(vec2 uv, vec3 vTangentDir){
	
	// We can adapt the layer count based on the view direction. If we are straight above the surface, we don't need many layers.
	float layersCount = mix(PARALLAX_MAX, PARALLAX_MIN, abs(vTangentDir.z));
	// Depth will vary between 0 and 1.
	float layerHeight = 1.0 / layersCount;
	float currentLayer = 0.0;
	// Initial depth at the given position.
	float currentDepth = texture(textureEffects, uv).z;
	
	// Step vector: in tangent space, we walk on the surface, in the (X,Y) plane.
	vec2 shift = PARALLAX_SCALE * vTangentDir.xy;
	// This shift corresponds to a UV shift, scaled depending on the height of a layer and the vertical coordinate of the view direction.
	vec2 shiftUV = shift / vTangentDir.z * layerHeight;
	vec2 newUV = uv;
	
	// While the current layer is above the surface (ie smaller than depth), we march.
	while (currentLayer < currentDepth) {
		// We update the UV, going further away from the viewer.
		newUV -= shiftUV;
		// Update current depth.
		currentDepth = texture(textureEffects,newUV).z;
		// Update current layer.
		currentLayer += layerHeight;
	}
	
	// Perform interpolation between the current depth layer and the previous one to refine the UV shift.
	vec2 previousNewUV = newUV + shiftUV;
	// The local depth is the gap between the current depth and the current depth layer.
	float currentLocalDepth = currentDepth - currentLayer;
	float previousLocalDepth = texture(textureEffects,previousNewUV).z - (currentLayer - layerHeight);
	
	// Interpolate between the two local depths to obtain the correct UV shift.
	return mix(newUV,previousNewUV,currentLocalDepth / (currentLocalDepth - previousLocalDepth));
}

float parallaxShadow(vec2 uv, vec3 lTangentDir){	
	float shadowMultiplier = 0.0;
	// Query the depth at the current shifted UV.
	float initialDepth = texture(textureEffects,uv).z;
	
	// Compute the adaptative number of sampling depth layers.
	float layersCount = mix(PARALLAX_MAX, PARALLAX_MIN, abs(lTangentDir.z));
	// Depth will vary between 0 and initialDepth, and we want to sample layersCount layers.
	float layerHeight = initialDepth / layersCount;
	
	// We will ray-march in the light direction, starting from the current point.
	// This shift corresponds to a UV shift, scaled depending on the height of
	// a layer and the vertical coordinate of the light direction.
	vec2 shiftUV = PARALLAX_SCALE * lTangentDir.xy / lTangentDir.z / layersCount;
	
	// Slightly bias the initial depth layer.
	float currentLayer = initialDepth - 0.1*layerHeight;
	float currentDepth = initialDepth;
	vec2 newUV = uv;
	float stepCount = 0.0;
	
	// While the depth is above 0.0, iterate and march along the light direction.
	while (currentLayer > 0.0  ) {
		
		// If we are below the surface
		if(currentDepth < currentLayer){
			// Increase the shadow factor:
			//	- the bigger the depth gap between the current sampling layer and the surface, the darker the shadow.
			//		-> (currentLayer - currentDepth)
			//	- the samples close to the starting point weight more than the further ones.
			//		-> (1.0 - stepCount/layersCount)
			shadowMultiplier += (currentLayer - currentDepth) * (1.0 - stepCount/layersCount);
		}
		// Increase the iteration count.
		stepCount += 1.0;
		// We update the UV, going further away from the viewer.
		newUV += shiftUV;
		// Update current depth.
		currentDepth = texture(textureEffects,newUV).z;
		// Update current layer.
		currentLayer -= layerHeight;
	}
	
	// Return the reversed factor, where 1 = no shadow and 0 = max shadow.
	return 1.0 - shadowMultiplier;
}

void main(){	
	// Combine view direction in tangent space.
	vec3 vTangentDir = normalize(IntangentSpaceView - IntangentSpacePosition);
	// Query UV offset.
	vec2 parallaxUV = parallax(Inuv, vTangentDir);
	// If UV are outside the texture ([0,1]), we discard the fragment.
	if(parallaxUV.x > 1.0 || parallaxUV.y  > 1.0 || parallaxUV.x < 0.0 || parallaxUV.y < 0.0){
		discard;
	}
	
	vec3 ambient;
	vec3 lightShading = shading(parallaxUV, uniforms.lightPosition.xyz, uniforms.lightShininess, uniforms.lightIs.rgb,ambient);
	
	// Compute parallax self-shadowing factor.
	// The light direction is computed, light.position.w == 0 if the light is directional, 1 else.
	vec3 lTangentDir = normalize(IntangentSpaceLight - uniforms.lightPosition.w * IntangentSpacePosition);
	float shadowParallax = parallaxShadow(parallaxUV, lTangentDir);
	
	// Shadow: combine the factor from the parallax self-shadowing with the factor from the shadow map.
	float shadowMultiplicator = shadow(InlightSpacePosition);
	shadowMultiplicator *= shadowParallax;
	
	// Mix the ambient color (always present) with the light contribution, weighted by the shadow factor.
	fragColor = vec4(ambient * uniforms.lightIa.rgb + shadowMultiplicator * lightShading, 0.0);	
}
