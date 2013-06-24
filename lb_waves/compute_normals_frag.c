#version 150
//--------------------------------------------------------------------------------
// compute_normals_frag.c
//
// Calculate normals for each vertex in the cellular automata. This is performed
// in a GLSL fragment shader
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------

uniform sampler2D inPos;
in vec2 tc;
uniform float deltaT;
uniform float deltaS;

out vec4 normal;

void main(void)
{
   vec2 upTC    = vec2(tc.s,          tc.t - deltaT);
   vec2 downTC  = vec2(tc.s,          tc.t + deltaT);
   vec2 leftTC  = vec2(tc.s - deltaS, tc.t) ;
   vec2 rightTC = vec2(tc.s + deltaS, tc.t);
   
   // Get the five positions
   vec4 upPos     = texture(inPos, upTC);
   vec4 downPos   = texture(inPos, downTC);
   vec4 rightPos  = texture(inPos, rightTC);
   vec4 leftPos   = texture(inPos, leftTC);
   vec4 centerPos = texture(inPos, tc);
   
   // Get the 4 vectors
   vec3 up    = normalize(upPos.xyz    - centerPos.xyz);
   vec3 down  = normalize(downPos.xyz  - centerPos.xyz);
   vec3 right = normalize(rightPos.xyz - centerPos.xyz);
   vec3 left  = normalize(leftPos.xyz  - centerPos.xyz);
   
   // Get the 4 vectors that are perpindicular to each
   // of the 4 faces
   vec3 upLeft    = cross(-left, up);
   vec3 upRight   = cross(-right, up);
   vec3 downLeft  = cross(-down, left);
   vec3 downRight = cross(-right, down);
   
   // Sum the vectors, normalize
   normal.xyz = normalize(upLeft + upRight + downLeft + downRight);
   
   normal.w = 0;
}

