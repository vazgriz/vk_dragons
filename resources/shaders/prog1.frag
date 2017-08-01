#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input: UV coordinates coming from the vertex shader
in vec2 uv; 

// Uniform: texture sampler
layout(binding = 1) uniform sampler2D texture1;

// Output: the fragment color
out vec3 fragColor;

void main(){
	// The output color is read from the texture, suing the UV coordinates.
	fragColor = texture(texture1, uv).rgb;
}
