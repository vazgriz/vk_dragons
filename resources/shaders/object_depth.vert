#version 450
#extension GL_ARB_separate_shader_objects : enable

// Attributes
layout(location = 0) in vec3 v;

layout(set = 1, binding = 0) uniform LightUniforms {
    mat4 viewProjection;
} lightUniforms;

layout(push_constant) uniform Model {
    mat4 matrix;
} model;

void main(){
    // We multiply the coordinates by the MVP matrix, and ouput the result.
    gl_Position = lightUniforms.viewProjection * model.matrix * vec4(v, 1.0);
}
