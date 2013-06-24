attribute vec4 pos;
attribute vec2 tc;
uniform mat4 proj;
uniform mat4 mv;
varying vec2 tcFrag;

void main(void)
{
   gl_Position = proj * mv * pos;
   tcFrag = tc;
}