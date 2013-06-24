#version 150

out vec4 fragColor;

float one_256 = 1.0 / 256;

void main(void)
{

   float dist = length(gl_PointCoord * 2 - 1);

   if(dist <= 1)
   {
      fragColor = vec4(0.09, one_256, 0.7, 0.75);
   }
   else
   {
      discard;
   }
}
