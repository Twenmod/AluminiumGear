#version 310 es
precision lowp float;                            
in vec2 v_texCoord; 
in vec3 v_position;

out vec4 FragColor;


struct Material {
    sampler2D diffuse;
};
uniform Material material;

void main()
{

    vec3 texColor = vec3(texture(material.diffuse, v_texCoord));

	FragColor = vec4(texColor, 1.0);
}