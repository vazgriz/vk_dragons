#version 450
#extension GL_ARB_separate_shader_objects : enable

// First attribute: vertex position
layout(location = 0) in vec3 v;
// Second attribute: uv coordinates
layout(location = 4) in vec2 t;

// Uniform: the camera matrix
layout(set = 0, binding = 0) uniform Uniforms {
    mat4 camProjection;
    mat4 camView;
    mat4 rotationOnlyView;
    mat4 camViewInverse;
    mat4 lightProjection;
    mat4 lightView;
} uniforms;

layout(push_constant) uniform Model {
    mat4 matrix;
    mat3 normalMatrix;
} model;

// Output: UV coordinates (for interpolation)
out vec2 uv; 

void main(){
	// We multiply the coordinates by the MVP matrix, and ouput the result.
	gl_Position = uniforms.camProjection * uniforms.camView * model.matrix * vec4(v, 1.0);

	uv = t;
}
