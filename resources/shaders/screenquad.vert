#version 450
#extension GL_ARB_separate_shader_objects : enable

// Attributes
layout(location = 0) in vec3 v;

// Output: UV coordinates
layout(location = 0) out vec2 uv;

void main(){	
	// We directly output the position.
	gl_Position = vec4(v, 1.0);
	
	// Output the UV coordinates computed from the positions.
	uv = v.xy * 0.5 + 0.5;	
}
