#version 150

in vec4 pos;

// Modelview and projection matrices. They need to be
// separate so that the pipeline can be broken up into
// separate transformation and projection stages
uniform mat4 mv;
uniform mat4 proj;

void main(void)
{
   gl_Position = proj * mv * pos;
}

