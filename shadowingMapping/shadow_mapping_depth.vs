//光空间转换顶点着色器，将顶点从世界坐标变换到光空间坐标
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 anormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 lightSpaceMatrix;//光空间变换矩阵
uniform mat4 model;//模型变换矩阵

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);
}