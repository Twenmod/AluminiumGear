#version 310 es
precision mediump float;                            
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


float length2(vec3 v) {
    return dot(v, v);  // Equivalent to v.x * v.x + v.y * v.y + v.z * v.z
}

void main()
{

    vec4 texColor = texture(material.diffuse, v_texCoord);

    vec3 viewDir = normalize(viewPos - v_position);

	vec4 result = vec4(0);

	//Ambient
	result += vec4(vec3(texColor),0);

	if (material.hasEmission)
		result += vec4(vec3(texture(material.emission,v_texCoord)),0);

	if (material.hasAlpha)
		FragColor = vec4(vec3(result),texture(material.alpha,v_texCoord).a);
	else
		FragColor = vec4(vec3(result),1);
}
