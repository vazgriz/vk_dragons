#version 450
#extension GL_ARB_separate_shader_objects : enable

// Attributes
layout(location = 0) in vec3 v;

// Uniform: the camera matrix
layout(set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
    mat4 rotationOnlyView;
} camera;

// Output: position in model space
out vec3 position;


void main(){
	// We multiply the coordinates by the MV matrix, and ouput the result.
	gl_Position = camera.projection * camera.rotationOnlyView * vec4(v, 1.0);
	position = v;
}
