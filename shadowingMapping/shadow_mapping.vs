//像素顶点着色器
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 lightSpaceMatrix;//光空间变换矩阵
uniform mat4 projection;//投影变换矩阵
uniform mat4 view;//视图变换矩阵
uniform mat4 model;//模型变换矩阵

out vec2 TexCoords;//纹理

out VS_OUT {
    vec3 FragPos;//世界坐标系中顶点位置
    vec3 Normal;//法向量
    vec2 TexCoords;//纹理坐标
    vec4 FragPosLightSpace;//光空间坐标系中顶点位置
} vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vs_out.FragPos = vec3(model * vec4(position,1.0));
    vs_out.Normal = transpose(inverse(mat3(model))) * normal;
    vs_out.TexCoords = texCoords;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
}