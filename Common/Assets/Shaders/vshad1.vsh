/*
here's a typical Vertex shader, we will use this as an exmple of how to load shaders
*/

precision mediump float;     

attribute vec3 a_position;   
attribute vec2 a_texCoord;   
uniform mat4 MVP;            
varying vec2 v_texCoord;     

void main()                  
{ 							  
 gl_Position =  MVP * vec4(a_position,1);
 v_texCoord = a_texCoord;  
}                          