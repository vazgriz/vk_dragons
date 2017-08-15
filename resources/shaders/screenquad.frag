#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input: UV coordinates
layout(location = 0) in vec2 uv;

// Uniforms: the texture, inverse of the screen size, FXAA flag.
layout(set = 0, binding = 0) uniform sampler2D screenTexture;

// Output: the fragment color
layout(location = 0) out vec4 fragColor;

void main(){	
	vec3 finalColor = texture(screenTexture,uv).rgb;
	fragColor = vec4(finalColor, 1.0);	
}
