mediump float one_256 = 1.0 / 256.0;

void main(void)
{
#if 1
   mediump vec2 coord = gl_PointCoord;
   coord.x *= 2.0;
   coord.y *= 2.0;
   coord.x -= 1.0;
   coord.y -= 1.0;
   
   mediump float dist = length(coord);
   if(dist <= 1.0)
   {
      gl_FragColor = vec4(0.09, one_256, 0.7, 0.75);
   }
   else
   {
      gl_FragColor = vec4(0);
   }
 #endif
   //   gl_FragColor = vec4(1);
}
