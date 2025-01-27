#version 310 es
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;  
uniform mat4 Model;
uniform mat3 NormalMatrix;
uniform mat4 VP;
out vec2 v_texCoord;
out vec3 v_position;
out vec3 v_normal;
out mat3 v_TBN;

void main()
{
	gl_Position = VP*Model * vec4(aPos,1);
	v_position = vec3(Model * vec4(aPos,1));
	v_texCoord = aTexCoords;
	v_normal = normalize(NormalMatrix * aNormal);

	//calc tbn matrix
    vec3 T = normalize(vec3(Model * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(Model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(Model * vec4(aNormal,    0.0)));
    v_TBN = mat3(T, B, N);
}