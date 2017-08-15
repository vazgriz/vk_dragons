#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input: UV coordinates coming from the vertex shader
layout(location = 0) in vec2 uv; 

// Uniform: texture sampler
layout(set = 1, binding = 0) uniform sampler2D texture1;

// Output: the fragment color
layout(location = 0) out vec4 fragColor;

void main(){
	// The output color is read from the texture, suing the UV coordinates.
	fragColor = texture(texture1, uv);
}
