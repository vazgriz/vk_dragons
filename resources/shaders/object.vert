#version 450
#extension GL_ARB_separate_shader_objects : enable

// Attributes
layout(location = 0) in vec3 v;
layout(location = 1) in vec3 n;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;
layout(location = 4) in vec3 binor;

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

// Output: tangent space matrix, position in view space and uv.
out INTERFACE {
    mat3 tbn;
	vec3 position; 
	vec2 uv;
	vec3 lightSpacePosition;
	vec3 modelPosition;
} Out ;


void main(){
	// We multiply the coordinates by the MVP matrix, and ouput the result.
	gl_Position = uniforms.camProjection * uniforms.camView * model.matrix * vec4(v, 1.0);

	Out.position = (uniforms.camView * model.matrix * vec4(v,1.0)).xyz;

	Out.uv = uv;

	// Compute the TBN matrix (from tangent space to view space).
	vec3 T = normalize(model.normalMatrix * tang);
	vec3 B = normalize(model.normalMatrix * binor);
	vec3 N = normalize(model.normalMatrix * n);
	Out.tbn = mat3(T, B, N);
	
	// Compute position in light space
	Out.lightSpacePosition = 0.5*(uniforms.lightProjection * uniforms.lightView * model.matrix * vec4(v,1.0)).xyz + 0.5;
	
	Out.modelPosition = v;
	
}
