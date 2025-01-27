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
out vec3 v_color;

uniform vec3 particleStartColor;
uniform vec3 particleEndColor;
uniform float particleSize;

uniform sampler2D particleDataTexture;
//HOW THE DATA IS STORED IN THE texture
/*
y coords  value
0         posX
1         posY
2         posZ
3         velX
4         velY
5         velZ
6         life
7         lifetime
*/

void main()
{
	int particleIndex = gl_InstanceID;

	ivec2 dataTexCoords = ivec2(particleIndex, 0);  //1D to 2D mapping for texture
	vec3 particlePos;
	particlePos.x = texelFetch(particleDataTexture,dataTexCoords+ivec2(0,0),0).r;
	particlePos.y = texelFetch(particleDataTexture,dataTexCoords+ivec2(0,1),0).r;
	particlePos.z = texelFetch(particleDataTexture,dataTexCoords+ivec2(0,2),0).r;

	float particleStage = 1.0-max(texelFetch(particleDataTexture,dataTexCoords+ivec2(0,6),0).r / texelFetch(particleDataTexture,dataTexCoords+ivec2(0,7),0).r,0.0);
	
	v_color = mix(particleStartColor, particleEndColor, particleStage);

	float scalar = (1.0 - particleStage)*particleSize;
	mat4 scalematrix = mat4(scalar, 0, 0, 0,
							0,scalar,  0, 0,
							0,0,scalar,   0,
							0,0, 0,       1);

	mat4 translation = mat4(1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 1, 0,
							particlePos.x, particlePos.y, particlePos.z, 1);

	gl_Position = VP* translation * scalematrix * vec4(aPos,1);
	v_position = vec3(Model * vec4(aPos,1));
	v_texCoord = aTexCoords;
	v_normal = normalize(NormalMatrix * aNormal);
}