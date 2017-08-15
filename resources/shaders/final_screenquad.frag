#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input: UV coordinates
layout(location = 0) in vec2 uv;

// Uniforms: the texture
layout(set = 0, binding = 0) uniform sampler2D screenTexture;

//Specialization constant. Set at pipeline creation time if software gamma correction is needed
layout(constant_id = 0) const bool enableGamma = false;
layout(constant_id = 1) const float gamma = 2.2;

// Output: the fragment color
layout(location = 0) out vec4 fragColor;

void main(){	
	vec3 finalColor = texture(screenTexture,uv).rgb;
	vec3 down = textureOffset(screenTexture,uv,ivec2(0,-1)).rgb;
	vec3 up = textureOffset(screenTexture,uv,ivec2(0,1)).rgb;
	vec3 left = textureOffset(screenTexture,uv,ivec2(-1,0)).rgb;
	vec3 right = textureOffset(screenTexture,uv,ivec2(1,0)).rgb;
	
	vec3 color = clamp(finalColor + 0.4*(4 * finalColor - down - up - left - right),0.0,1.0);
	
	if (enableGamma) {
		color = pow(color, vec3(1.0 / gamma));
	}

	fragColor = vec4(color, 1.0);	
}
