varying mediump vec2 tcFrag;

void main(void)
{
   gl_FragColor = vec4(tcFrag, 0, 1);
}