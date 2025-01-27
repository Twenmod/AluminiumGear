#version 310 es
precision lowp float;                            
in vec2 v_texCoord; 
in vec3 v_position;
in vec3 v_normal;

out vec4 FragColor;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse;
	bool hasSpec;
    sampler2D specular;
	bool hasEmission;
	sampler2D emission;
	bool hasAlpha;
	sampler2D alpha;
};
uniform Material material;


//Lighting
uniform vec3 ambient;

struct DirLight {
    vec3 direction;
    vec3 diffuse;
    vec3 specular;
};
#define MAX_DIR_LIGHTS 1
uniform DirLight dirLight[MAX_DIR_LIGHTS];

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 diffuse;
    vec3 specular;
};  
#define MAX_POINT_LIGHTS 9
uniform PointLight pointLights[MAX_POINT_LIGHTS];


const float MAX_LIGHT_DISTANCE_SOFT = (30.0*30.0);
const float MAX_LIGHT_DISTANCE_HARD = (50.0*50.0);


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor);
vec3 CalcCubeReflectRay(vec3 pos, vec3 size);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor);

float length2(vec3 v) {
    return dot(v, v);  // Equivalent to v.x * v.x + v.y * v.y + v.z * v.z
}

void main()
{

    vec4 texColor = texture(material.diffuse, v_texCoord);

    vec3 viewDir = normalize(viewPos - v_position);

	vec4 result = vec4(0);
	if (material.hasAlpha) result.a = texture(material.alpha,v_texCoord).a;
	else result.a = 1.0;

	//Lighting
    for(int i = 0; i < MAX_DIR_LIGHTS; i++)
		result += vec4(CalcDirLight(dirLight[i], v_normal, viewDir,vec3(texColor)),0);
	for(int i = 0; i < MAX_POINT_LIGHTS; i++)
		result += vec4(CalcPointLight(pointLights[i], v_normal, v_position, viewDir,vec3(texColor)),0);

	//Ambient
	result += vec4(ambient * vec3(texColor),0);

	if (material.hasEmission)
		result += vec4(vec3(texture(material.emission,v_texCoord)),0);

	FragColor = vec4(result);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor) {
    vec3 lightDir = -light.direction;
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(lightDir, normal);

    // combine results
    vec3 diffuse  = light.diffuse  * diff * texColor;
    return (diffuse);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor)
{
	float viewDist = length2(viewPos - fragPos);
	float distScale = max(min(1.0 - ((viewDist - MAX_LIGHT_DISTANCE_SOFT) / MAX_LIGHT_DISTANCE_HARD), 1.0),0.0);
    // attenuation
    float dist    = length2(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + (light.quadratic) * dist); // No linear falloff on pi since that is more expensive     

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading

    // combine results
    vec3 diffuse  = light.diffuse  * diff * texColor;
    diffuse  *= attenuation;
	diffuse *= distScale;
    return (diffuse);
} 