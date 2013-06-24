#version 150

uniform sampler2D pos;
in vec2 posIdx;

uniform mat4 mv;
uniform mat4 proj;

void main(void)
{
   // Read the position from the texture map
   gl_Position = proj * mv * texture(pos, posIdx);
}

