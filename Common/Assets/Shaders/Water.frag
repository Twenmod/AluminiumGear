#version 310 es
precision mediump float;                            
in vec2 v_texCoord; 
in vec3 v_position;
in vec3 v_normal;

out vec4 FragColor;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse;
};
uniform Material material;

float baseNoiseValue = 0.4;

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

//Animation
uniform float timer;

//reflection
uniform samplerCube cubemap[2];
float reflectAmount = 1.0;
uniform bool reflective;
uniform vec3 probeHalfSize;
uniform vec3 reflectProbePos;

uniform vec3 probeHalfSize2;
uniform vec3 reflectProbePos2;

float calculateBoxMixFactor(vec3 position, vec3 probePos, vec3 halfSize) {
    vec3 distToProbeCenter = abs(position - probePos);

    vec3 normalizedDist = distToProbeCenter / halfSize;

    vec3 clampedDist = clamp(normalizedDist, 0.0, 1.0);

    float mixFactor = max(clampedDist.x, max(clampedDist.y, clampedDist.z));

    return mixFactor;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor);
vec3 CalcCubeReflectRay(vec3 pos, vec3 size);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor);

float length2(vec3 v) {
    return dot(v, v);  // Equivalent to v.x * v.x + v.y * v.y + v.z * v.z
}

void main()
{

	float color1 = texture(material.diffuse, v_texCoord + vec2(1)*timer).r;
	float color2 = texture(material.diffuse, v_texCoord + vec2(-0.5)*timer+0.74).r; // 0.74 is a Random number for offset
	vec4 texColor = vec4(0.3,0.3,0.8,1.0)*min(color1*color2+baseNoiseValue,1.0);

    vec3 viewDir = normalize(viewPos - v_position);

	vec4 result = vec4(0);

	//Lighting
    for(int i = 0; i < amountOfDirLights; i++)
		result += vec4(CalcDirLight(dirLight[i], v_normal, viewDir,vec3(texColor)),0);
	for(int i = 0; i < amountOfPointLights; i++)
		result += vec4(CalcPointLight(pointLights[i], v_normal, v_position, viewDir,vec3(texColor)),0);

	//Ambient
	result += vec4(ambient * vec3(texColor),0);

	if (reflective) {
		float amount = reflectAmount;
		float mixFactor1 = 1.0 - calculateBoxMixFactor(v_position, reflectProbePos, probeHalfSize);
		float mixFactor2 = 1.0 - calculateBoxMixFactor(v_position, reflectProbePos2, probeHalfSize2);
		vec3 ray = CalcCubeReflectRay(reflectProbePos, probeHalfSize);

		//rotate ray to approximate normal;
		vec3 diff = v_normal - vec3(0,1,0);
		float refraction = 32.0;
		ray += diff*refraction;

		vec3 closestProbe = vec3(0);
		if (ray != vec3(0)) closestProbe = texture(cubemap[0], ray).rgb * mixFactor1;
		ray = CalcCubeReflectRay(reflectProbePos2, probeHalfSize2);
		vec3 secondProbe = vec3(0);
		if (ray != vec3(0)) secondProbe = texture(cubemap[1], ray).rgb * mixFactor2;

		// Calculate the mix factor for both probes

		float totalWeight = mixFactor1 + mixFactor2;
		mixFactor2 /= totalWeight;
		float a = clamp(mixFactor2,0.0,1.0);
		a *= clamp(length2(reflectProbePos - viewPos) / length2(reflectProbePos2 - viewPos),0.0,1.0);
		
		result += vec4(mix(closestProbe,secondProbe,a),0)*amount;
	}

	FragColor = vec4(vec3(result),texColor.b+0.25);
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
		//Which does something similar but not entirely

		// get ray from camera to pos in world
		vec3 ray = v_position-viewPos;
		ray = normalize(ray);


		// gets min / max intersections with ray and cube
		// (not sure about this vector division or how it works tbh)
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
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = max(dot(viewDir, reflectDir), 0.0);
	spec *= spec;
    // attenuation
    float dist    = length2(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * sqrt(dist) + 
  			     light.quadratic * (dist));    
    // combine results
    vec3 diffuse  = light.diffuse  * diff * texColor;
    diffuse  *= attenuation;
    return (diffuse);
} 