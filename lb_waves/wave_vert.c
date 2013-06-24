#version 150
//--------------------------------------------------------------------------------
// wave_vert.c
//
// Vertex shader for the waves
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------

uniform sampler2D pos;
uniform sampler2D normals;
in vec2 posIdx;
uniform mat4 mv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

//uniform mat4 invTP;

out vec3 lightVector;
out vec3 normalVector;
out vec3 halfwayVector;


const vec3 lightPosition = vec3(10,10,0);

void main()
{
   vec3 normal = texture(normals, posIdx).xyz;
   vec4 modelPos = texture(pos, posIdx);
   modelPos.x = posIdx.s * 160 - 80;
   modelPos.z = posIdx.t * 160 - 80;

   gl_Position = view * model * modelPos;
   
   lightVector = normalize((view * vec4(lightPosition, 1.0)).xyz - gl_Position.xyz);
   normalVector = (inverse(transpose(view * model)) * vec4(normal, 0.0)).xyz;
   halfwayVector = lightVector + normalize(-gl_Position.xyz);
   
   gl_Position = proj* gl_Position;

}
