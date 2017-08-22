#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec2 fragColor;

void main() {
	fragColor = vec2(gl_FragCoord.z, gl_FragCoord.z * gl_FragCoord.z);
}