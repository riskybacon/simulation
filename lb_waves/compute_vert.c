#version 150
//--------------------------------------------------------------------------------
// compute_vert.c
//
// Computation vertex shader for both the cellular automata update and the
// normals calculation fragment shaders.
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------

in vec4 pos;
in vec2 texcoord;
out vec2 tc;

void main(void)
{
   gl_Position = pos;
   tc = texcoord;
}

