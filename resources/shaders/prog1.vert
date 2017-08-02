#version 450
#extension GL_ARB_separate_shader_objects : enable

// First attribute: vertex position
layout(location = 0) in vec3 v;
// Second attribute: uv coordinates
layout(location = 4) in vec2 t;

// Uniform: the camera matrix
layout(binding = 0) uniform Camera {
    mat4 viewProjection;
} camera;

// Output: UV coordinates (for interpolation)
out vec2 uv; 

void main(){
	// We multiply the coordinates by the MVP matrix, and ouput the result.
	gl_Position = camera.viewProjection * vec4(v, 1.0);

	uv = t;
}
