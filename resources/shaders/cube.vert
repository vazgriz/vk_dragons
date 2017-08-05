#version 450
#extension GL_ARB_separate_shader_objects : enable

// Attributes
layout(location = 0) in vec3 v;

// Uniform: the camera matrix
layout(set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
} camera;

// Output: position in model space
out INTERFACE {
	vec3 position;
} Out ;


void main(){
	// We multiply the coordinates by the MV matrix, and ouput the result.
	gl_Position = camera.projection * camera.view * vec4(v, 1.0);

	Out.position = v;
}
