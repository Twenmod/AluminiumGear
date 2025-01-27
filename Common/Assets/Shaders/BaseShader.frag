#version 310 es
precision mediump float;                            
in vec2 v_texCoord; 
in vec3 v_position;
in vec3 v_normal;
in mat3 v_TBN;

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
	bool hasNormal;
	sampler2D normal;
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

//reflection
uniform samplerCube cubemap[2];
float reflectAmount = 1.5;
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
vec3 CalcCubeReflectRay(vec3 pos, vec3 size, vec3 normal);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor);

float length2(vec3 v) {
    return dot(v, v);  // Equivalent to v.x * v.x + v.y * v.y + v.z * v.z
}


uint state = 0u;

uint lcg_rand()
{
    state = state * 1664525u + 1013904223u; // Basic LCG formula
    return state;
}
uint randInt(uint range)
{
	return lcg_rand() % range;
}

// Function to generate a random float between [0, 1]
float rand()
{
	return float(randInt(255u)) / 255.0;
}


uint arithmetic_hash(uint h)
{
    h = (h * 1664525u) + 1013904223u; 
    h = (h * 22695477u) + 1u;        
    h = (h * 1103515245u) + 12345u;    
    return h;
}

void main()
{

	vec3 normal = v_normal;

	if (material.hasNormal) {
		//Calculate frag normal based on map
		normal = texture(material.normal, v_texCoord).rgb;
		normal = normal * 2.0 - 1.0;   
		normal = normalize(v_TBN * normal); // transform from tangent to world space
	}

//	FragColor = vec4((normal*2.0)-vec3(1.0),1);
//	return;

	state = arithmetic_hash(uint(abs(v_position.x*v_position.y*gl_FragCoord.x*gl_FragCoord.y)));

    vec4 texColor = texture(material.diffuse, v_texCoord);

    vec3 viewDir = normalize(viewPos - v_position);

	vec4 result = vec4(0);

	//Lighting
    for(int i = 0; i < amountOfDirLights; i++)
		result += vec4(CalcDirLight(dirLight[i], normal, viewDir,vec3(texColor)),0);
	for(int i = 0; i < amountOfPointLights; i++)
		result += vec4(CalcPointLight(pointLights[i], normal, v_position, viewDir,vec3(texColor)),0);

	//Ambient
	result += vec4(ambient * vec3(texColor),0);

	if (reflective) {
		float angle = 1.0 - max(dot(normal,viewDir),0.0);
		float amount = min(reflectAmount*angle,1.0);
		float mixFactor1 = 1.0 - calculateBoxMixFactor(v_position, reflectProbePos, probeHalfSize);
		float mixFactor2 = 1.0 - calculateBoxMixFactor(v_position, reflectProbePos2, probeHalfSize2);
		vec3 ray = CalcCubeReflectRay(reflectProbePos, probeHalfSize, normal);

		float refraction = 1.0;

		//rotate ray to approximate normal;
		vec3 diff = normal - viewDir;
		ray += diff*refraction;

//		//Glossiness
		float glossiness = 1.4;

		vec3 closestProbe = vec3(0);
		if (ray != vec3(0)) {
			closestProbe = texture(cubemap[0], ray+ vec3(rand()-0.5,rand()-0.5,rand()-0.5)*glossiness).rgb * amount * mixFactor1;
			closestProbe += texture(cubemap[0], ray+ vec3(rand()-0.5,rand()-0.5,rand()-0.5)*glossiness).rgb * amount * mixFactor1;
			closestProbe /=2.0;
		}
		ray = CalcCubeReflectRay(reflectProbePos2, probeHalfSize2, normal);

		//rotate ray to approximate normal;
		diff = normal - viewDir;
		ray += diff*refraction;

		vec3 secondProbe = vec3(0);
		if (ray != vec3(0)) {
			secondProbe = texture(cubemap[1], ray+ vec3(rand()-0.5,rand()-0.5,rand()-0.5)*glossiness).rgb * amount * mixFactor2;
			secondProbe += texture(cubemap[1], ray+ vec3(rand()-0.5,rand()-0.5,rand()-0.5)*glossiness).rgb * amount * mixFactor2;
			secondProbe /=2.0;
		}
		// Calculate the mix factor for both probes

		float totalWeight = mixFactor1 + mixFactor2;
		mixFactor2 /= totalWeight;
		float a = clamp(mixFactor2,0.0,1.0);
		a *= clamp(length2(reflectProbePos - viewPos) / length2(reflectProbePos2 - viewPos),0.0,1.0);
		
		result += vec4(mix(closestProbe,secondProbe,a),0);
	}

	if (material.hasEmission)
		result += vec4(vec3(texture(material.emission,v_texCoord)),0);

	if (material.hasAlpha)
		FragColor = vec4(vec3(result),texture(material.alpha,v_texCoord).a);
	else
		FragColor = vec4(vec3(result),1);
}

vec3 CalcCubeReflectRay(vec3 pos, vec3 halfSize, vec3 normal) {
	vec3 absNormal = round(abs(normal));

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

		if (absNormal.x > absNormal.z && absNormal.x > absNormal.y) {
		ray.x = -ray.x;
		}
		if (absNormal.z > absNormal.x && absNormal.z > absNormal.y) ray.z = -ray.z;

		//vec3 reflec = reflect(-viewDir, normalize(normal));
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
    vec3 specular = light.specular * spec * vec3(texture(material.specular, v_texCoord));
    return (diffuse + specular);
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
    vec3 specular = light.specular * spec * vec3(texture(material.specular, v_texCoord));
    diffuse  *= attenuation;
    specular *= attenuation;
    return (diffuse + specular);
} 