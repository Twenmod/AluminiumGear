#version 310 es
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
uniform mat4 Model;
uniform mat3 NormalMatrix;
uniform mat4 VP;
out vec2 v_texCoord;
out vec3 v_position;
out vec3 v_normal;

void main()
{
	gl_Position = VP*Model * vec4(aPos,1);
	v_position = vec3(Model * vec4(aPos,1));
	v_texCoord = aTexCoords;
	v_normal = normalize(NormalMatrix * aNormal);
}