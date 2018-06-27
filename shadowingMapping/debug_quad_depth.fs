//四边形片段着色器
#version 330 core
out vec4 color;
in vec2 TexCoords;

uniform sampler2D depthMap;//阴影纹理

uniform float near_plane;//近平面
uniform float far_plane;//远平面

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{             
    float depthValue = texture(depthMap, TexCoords).r;//深度值
    color = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // 透视投影
    //color = vec4(vec3(depthValue), 1.0);// 正交投影
}