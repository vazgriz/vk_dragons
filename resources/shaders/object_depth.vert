#version 450
#extension GL_ARB_separate_shader_objects : enable

// Attributes
layout(location = 0) in vec3 v;

// Uniform: the MVP, MV and normal matrices
layout(set = 0, binding = 0) uniform Uniforms {
    mat4 projection;
    mat4 view;
    mat4 rotationOnlyView;
    mat4 lightProjection;
    mat4 lightView;
} uniforms;

void main(){
	// We multiply the coordinates by the MVP matrix, and ouput the result.
	gl_Position = uniforms.lightProjection * uniforms.lightView * vec4(v, 1.0);
}
