#version 150

in vec4 pos;
in vec2 texcoord;
out vec2 tc;

void main(void)
{
   gl_Position = pos;
   tc = texcoord;
}
