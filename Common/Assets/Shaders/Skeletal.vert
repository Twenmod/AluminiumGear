#version 310 es

//Modifed from https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;
	
uniform mat4 Model;
uniform mat3 NormalMatrix;
uniform mat4 VP;
	
const int MAX_BONES = 65;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
	
out vec2 v_texCoord;
out vec3 v_position;
out vec3 v_normal;


void main()
{
    mat4 MVP = VP * Model;

    vec4 totalPosition = vec4(0.0f);
	vec3 totalNormal = aNormal;
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(aPos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos,1.0f);
        totalPosition += localPosition * weights[i];
        totalNormal = mat3(finalBonesMatrices[boneIds[i]]) * aNormal;
    }
		
    gl_Position =  MVP * totalPosition;
	v_position = vec3(Model * vec4(aPos,1));
    v_texCoord = aTexCoords;
	v_normal = normalize(NormalMatrix * totalNormal);
}