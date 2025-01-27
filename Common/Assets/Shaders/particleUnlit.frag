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

void main()
{

    vec4 texColor = vec4(v_color,1.0);


	vec4 result = texColor;

	if (material.hasAlpha)
		FragColor = vec4(vec3(result),texture(material.alpha,v_texCoord).a);
	else
		FragColor = vec4(vec3(result),1);
}