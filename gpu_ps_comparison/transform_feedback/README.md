Gravity simulation using OpenGL's transform feedback buffer. There is a
single gravity source in the scene. New positions are calculated using
Newton's 2nd law with RK4.

The computation is performed using the transform feedback buffer. This means
that new positions and velocities are calcuated in a vertex shader which
outputs the new positions and velocities. The results are stored in 
OpenGL buffer objects and are not passed on to the rest of the OpenGL pipeline.

The next pass uses this output and renders the result.

This was the slowest method that was tested. The other two methods were:

1) Vertex Texture Fetch
2) Copy to PBO
