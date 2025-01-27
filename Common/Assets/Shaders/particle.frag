#version 310 es
precision mediump float;                            
in vec2 v_texCoord; 
in vec3 v_position;
in vec3 v_normal;
in vec3 v_color;

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

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor);
vec3 CalcCubeReflectRay(vec3 pos, vec3 size);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor);

float length2(vec3 v) {
    return dot(v, v);  // Equivalent to v.x * v.x + v.y * v.y + v.z * v.z
}

void main()
{

    vec4 texColor = vec4(v_color,1.0);

    vec3 viewDir = normalize(viewPos - v_position);

	vec4 result = vec4(0);

	//Lighting
    for(int i = 0; i < amountOfDirLights; i++)
		result += vec4(CalcDirLight(dirLight[i], v_normal, viewDir,vec3(texColor)),0);
	for(int i = 0; i < amountOfPointLights; i++)
		result += vec4(CalcPointLight(pointLights[i], v_normal, v_position, viewDir,vec3(texColor)),0);

	//Ambient
	result += vec4(ambient * vec3(texColor),0);

	if (material.hasEmission)
		result += vec4(vec3(texture(material.emission,v_texCoord)),0);

	if (material.hasAlpha)
		FragColor = vec4(vec3(result),texture(material.alpha,v_texCoord).a);
	else
		FragColor = vec4(vec3(result),1);
}

vec3 CalcCubeReflectRay(vec3 pos, vec3 halfSize) {
	vec3 absNormal = round(abs(v_normal));

	vec3 boxMin = pos - halfSize;
	vec3 boxMax = pos + halfSize;

	if (v_position.x >= boxMin.x &&
		v_position.x <= boxMax.x &&
		v_position.y >= boxMin.y &&
		v_position.y <= boxMax.y &&
		v_position.z >= boxMin.z &&
		v_position.z <= boxMax.z ) 
	{

		//Generate box
		// Calculate the center of the box using the facePosition and normal
		vec3 center = pos - (-(v_position-pos) * absNormal * halfSize*2.0);

		// Calculate min and max of the box
		boxMin = center - halfSize;
		boxMax = center + halfSize;
    

		//Modifed from https://forum.derivative.ca/t/parallax-corrected-cubemap-shaders-glsl/299290
		//Which does an interior shader which is the same concept but executed differently

		// get ray from camera to pos in world
		vec3 ray = v_position-viewPos;
		ray = normalize(ray);

		// gets min / max intersections with ray and cube
		vec3 planeIntersect1 = (boxMax - viewPos) / ray;
		vec3 planeIntersect2 = (boxMin - viewPos) / ray;

		// pick the furthest intersection
		vec3 furthestPlane = max(planeIntersect1, planeIntersect2);

		// get the distance to closest intersection on this cube plane
		float dist = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);
		

		// use this to recover the final intersected world space
		vec3 intersectedWorldSpacePos = viewPos + ray * dist;
   
   		if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) intersectedWorldSpacePos.y += (pos.y - intersectedWorldSpacePos.y)*2.0;

		// now get the ray in cubemap coords
		ray = intersectedWorldSpacePos - pos;


		if (absNormal.x > absNormal.z && absNormal.x > absNormal.y) ray.x = -ray.x;
		if (absNormal.z > absNormal.x && absNormal.z > absNormal.y) ray.z = -ray.z;

		//vec3 reflec = reflect(-viewDir, normalize(v_normal));
		return ray;
	}else return vec3(0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor) {
    vec3 lightDir = -light.direction;
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = max(dot(viewDir, reflectDir), 0.0);
	spec *= spec;
    // combine results
    vec3 diffuse  = light.diffuse  * diff * texColor;
    return (diffuse);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor)
{
    float dist    = length2(light.position - fragPos); 
    float attenuation = 1.0 / (light.constant + light.linear * sqrt(dist) + 
  			     light.quadratic * (dist));
	if (attenuation <= 0.01) return (vec3(0));
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = max(dot(viewDir, reflectDir), 0.0);
	spec *= spec;
    // attenuation
    // combine results
    vec3 diffuse  = light.diffuse  * diff * texColor;
    diffuse  *= attenuation;
    return (diffuse);
} 