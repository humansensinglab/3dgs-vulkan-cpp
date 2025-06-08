#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 rotation;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
   
    vec3 rotated3D = (ubo.rotation * vec4(inPosition * 0.15, 1.0)).xyz;
    gl_Position = vec4(rotated3D.xy vec2(0.7, 0.7), 0.0, 1.0);
    
    fragColor = inColor;
}