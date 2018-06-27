//像素片段着色器
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

//计算阴影元素
float ShadowCalculation(vec4 fragPosLightSpace,vec3 normal,vec3 lightDir)
{
    //执行透视除法(当使用透视投影时用到)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;//返回了片元在光空间的-1到1的范围内的坐标
    //变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    //取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    //取得当前片元在光源视角下的深度
    float currentDepth = projCoords.z;
    //检查当前片元是否在阴影中
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);//偏移量(运用阴影偏移)
    //float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    //PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    //当点比光的远平面还要远时，阴影不起作用
    if(projCoords.z > 1.0){
    	shadow = 0.0;
    }

    return shadow;
}

void main()
{             
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;//物体颜色
    vec3 normal = normalize(fs_in.Normal);//法向量
    vec3 lightColor = vec3(1.0);//光源颜色
    //*********Blinn-Phong光照模型*********
    //环境光
    vec3 ambient = 0.15 * color;//计算环境光照强度
    //漫反射
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);//计算入射光向量（片段位置指向光源位置）
    float diff = max(dot(lightDir, normal), 0.0);//计算漫反射分量，为入射光向量与法向量的余弦值（大于0）
    vec3 diffuse = diff * lightColor;//计算漫反射光照强度
    //镜面反射
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);//计算观察向量（片段位置指向观察者所在位置）
    vec3 reflectDir = reflect(-lightDir, normal);//计算反射光向量
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;//计算镜面反射向量
    //计算阴影
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace,normal,lightDir);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
    FragColor = vec4(lighting,1.0f);    
}