attribute vec4 pos;

uniform mat4 mv;
uniform mat4 proj;

void main(void)
{
   gl_PointSize = 2.0;
   gl_Position = proj * mv * pos;
}

