#version 310 es
precision mediump float;                            
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube cubemap;

void main()
{    
    FragColor = vec4(vec3(texture(cubemap, TexCoords)),1);
}