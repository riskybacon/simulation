#version 150

in vec4 pos;

uniform mat4 mv;
uniform mat4 proj;

void main(void)
{
   gl_Position = proj * mv * pos;
}

