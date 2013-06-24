Gravity simulation performed using OpenGL FBOs and a fragment shader.
There is a single gravity source in the scene. New positions are calculated
using Newton's 2nd law with RK4.

The computation is performed in a fragment shader and the resulting output
is stored in an OpenGL texture map. The render pass retrieves the particle 
positions from the texture map using a technique called "vertex texture fetch"

This was the best performing method.
