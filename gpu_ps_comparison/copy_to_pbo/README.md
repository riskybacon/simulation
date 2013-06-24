Gravity simulation performed using OpenGL FBOs and a fragment shader.
There is ingle gravity source in the scene. New positions are calculated using
Newton's 2nd law with RK4.

The computation is performed in a fragment shader and the resulting output
is copied from an OpenGL texture map to a GL_ARRAY_BUFFER. The hope is that 
OpenGL will not actually perform a copy, but instead will set up a buffer
that points to the same place in memory as the texture map.

This was the middle performing method that was tested:

1) Vertex Texture Fetch
2) Transform Feedback Buffer
