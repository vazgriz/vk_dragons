#version 450
#extension GL_ARB_separate_shader_objects : enable

// Attributes
layout(location = 0) in vec3 v;

// Uniform: the MVP, MV and normal matrices
layout(set = 0, binding = 0) uniform CamUniforms {
	mat4 camProjection;
	mat4 camView;
	mat4 rotationOnlyView;
	mat4 camViewInverse;
} camUniforms;

layout(set = 1, binding = 0) uniform LightUniforms {
	mat4 lightProjection;
	mat4 lightView;
	vec4 lightPosition;
	vec4 lightIa;
	vec4 lightId;
	vec4 lightIs;
	float lightShininess;
} lightUniforms;

layout(push_constant) uniform Model {
	mat4 matrix;
} model;

void main(){
	// We multiply the coordinates by the MVP matrix, and ouput the result.
	gl_Position = lightUniforms.lightProjection * lightUniforms.lightView * model.matrix * vec4(v, 1.0);
}
