#version 450
#extension GL_ARB_separate_shader_objects : enable

// Attributes
layout(location = 0) in vec3 v;

layout(set = 1, binding = 0) uniform LightUniforms {
    mat4 viewProjection;
} lightUniforms;

layout(set = 2, binding = 0) uniform ModelUniforms {
    mat4 mvp;
    mat4 mv;
    mat4 lightMVP;
} modelUniforms;

layout(push_constant) uniform Model {
    mat4 matrix;
} model;

void main(){
    // We multiply the coordinates by the MVP matrix, and ouput the result.
    gl_Position = modelUniforms.lightMVP * vec4(v, 1.0);
}
