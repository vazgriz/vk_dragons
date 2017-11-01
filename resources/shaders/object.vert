#version 450
#extension GL_ARB_separate_shader_objects : enable

// Attributes
layout(location = 0) in vec3 v;
layout(location = 1) in vec3 n;
layout(location = 2) in vec3 tang;
layout(location = 3) in vec3 binor;
layout(location = 4) in vec2 uv;

layout(set = 0, binding = 0) uniform CamUniforms {
    mat4 viewProjection;
    mat4 projection;
    mat4 view;
    mat4 rotationOnlyView;
    mat4 viewInverse;
} camUniforms;

layout(set = 1, binding = 0) uniform LightUniforms {
    mat4 viewProjection;
} lightUniforms;

layout(push_constant) uniform Model {
    mat4 matrix;
    mat3 normalMatrix;
} model;

// Output: tangent space matrix, position in view space and uv.
layout(location = 0) out mat3 Outtbn;
layout(location = 3) out vec3 Outposition; 
layout(location = 4) out vec2 Outuv;
layout(location = 5) out vec3 OutlightSpacePosition;

void main(){
    // We multiply the coordinates by the MVP matrix, and ouput the result.
    gl_Position = camUniforms.viewProjection * model.matrix * vec4(v, 1.0);

    Outposition = (camUniforms.view * model.matrix * vec4(v,1.0)).xyz;

    Outuv = uv;

    // Compute the TBN matrix (from tangent space to view space).
    vec3 T = normalize(model.normalMatrix * tang);
    vec3 B = normalize(model.normalMatrix * binor);
    vec3 N = normalize(model.normalMatrix * n);
    Outtbn = mat3(T, B, N);
    
    // Compute position in light space
    vec4 lightPosition = lightUniforms.viewProjection * model.matrix * vec4(v,1.0);
    OutlightSpacePosition.xy = 0.5 * lightPosition.xy + 0.5;
    OutlightSpacePosition.z = lightPosition.z;
}
