#version 310 es
precision mediump float;                            

in vec2 v_texCoord; 
in vec3 v_position;
in vec3 v_normal;

out vec4 FragColor;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
	sampler2D emission;
};
uniform Material material;

//Lighting
uniform vec3 ambient;

struct DirLight {
    vec3 direction;
    vec3 diffuse;
    vec3 specular;
};
#define MAX_DIR_LIGHTS 2
uniform DirLight dirLight[MAX_DIR_LIGHTS];
uniform int amountOfDirLights;

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 diffuse;
    vec3 specular;
};  
#define MAX_POINT_LIGHTS 16
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int amountOfPointLights;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);  

void main()
{
    vec4 texColor = texture(material.diffuse, v_texCoord);

    vec3 viewDir = normalize(viewPos - v_position);

	vec4 result = vec4(0);

	//Lighting
    for(int i = 0; i < amountOfDirLights; i++)
		result += vec4(CalcDirLight(dirLight[i], v_normal, viewDir),0);
	for(int i = 0; i < amountOfPointLights; i++)
		result += vec4(CalcPointLight(pointLights[i], v_normal, v_position, viewDir),0);

	//Ambient
	result += vec4(ambient * vec3(texture(material.diffuse,v_texCoord)),0);

	FragColor = vec4(vec3(result),1);
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = -light.direction;
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = max(dot(viewDir, reflectDir), 0.0);
	spec *= spec;
    // combine results
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, v_texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, v_texCoord));
    return (diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = max(dot(viewDir, reflectDir), 0.0);
	spec *= spec;
    // attenuation
    float dist    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + 
  			     light.quadratic * (dist * dist));    
    // combine results
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, v_texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, v_texCoord));
    diffuse  *= attenuation;
    specular *= attenuation;
    return (diffuse + specular);
} 