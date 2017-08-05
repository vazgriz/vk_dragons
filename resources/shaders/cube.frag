#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input: position in model space
in INTERFACE {
	vec3 position; 
} In ;

layout(set = 1, binding = 0) uniform samplerCube textureCubeMap;

// Output: the fragment color
layout(location = 0) out vec3 fragColor;

void main(){
	fragColor = texture(textureCubeMap,In.position).rgb;
}
