#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include <sys/time.h>
#include <unistd.h>

#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtx/std_based_type.hpp>
#include <glm/virtrev/xstream.hpp>       // ostream
#include <glm/gtc/matrix_transform.hpp>  // rotation, translation, scale, etc
#include <glm//gtc/type_ptr.hpp>         // value_ptr

#include <opengl.h>
#include <shader.h>
#include <trackball.h>

using std::vector;
using std::string;
using std::unique_ptr;
using namespace glm;

string                  _updateVertFile;     //< File name for the PS update shader
string                  _updateFragFile;     //< File name for the PS update shader
string                  _renderVertFile;     //< File name for the PS vertex shader
string                  _renderFragFile;     //< File name for the PS fragment shader
unique_ptr<GL::Program> _updateProg;         //< Pointer to the PS update program
unique_ptr<GL::Program> _renderProg;         //< Pointer to the PS rendering program
GLuint                  _pVAO;               //< Vertex array object for the positions
GLuint                  _pBO;                //< Buffer object for the positions


GLuint                  _computeVAO;         //< Vertex array object for the compute quad
GLuint                  _computePosBuf;      //< Buffer object for compute quad positions
GLuint                  _computeTexCoordBuf; //< Buffer object for compute quad texcoords

vector<GLuint>          _posTexID;           //< Texture IDs for the position textures
vector<GLuint>          _velTexID;           //< Texture IDs for the velocity textures
vector<GLuint>          _fboID;              //< Frame buffer object handles
unsigned int            _srcCompute;         //< Source compute texture
unsigned int            _dstCompute;         //< Destination compute texture

bool                    _running = true;     //< true if the program should continue running

// Particle data
vector<vec4>            _positions;
vector<vec4>            _velocities;
size_t                  _particleWidth;      //< Width of the particle data texture
size_t                  _particleHeight;     //< Height of the particle data texture

// Track the framerate
unsigned long           _numFrames;          //< Number of frames drawn
timeval                 _startTime;          //< Start time of program
timeval                 _endTime;            //< End time of program

// User interaction
unique_ptr<Trackball>   _trackball;
// This keeps the user from zooming really far in or out and then having to spend
// time to undo their actions. Zooming is clamped to a max and min value by
// checking the delta of the mouse wheel each time it changes.
float                   _mouseWheelPrev;     //< Previous mouse wheel position;
float                   _zoom;               //< Current mouse wheel zoom in/out factor
float                   _zoomMax;            //< Max allowed mouse wheel zoom
float                   _zoomMin;            //< Min allowed mouse wheel zoom
bool                    _tracking;

/**
 * Clean up and exit
 *
 * @param exitCode      The exit code, eg, EXIT_SUCCESS or EXIT_FAILURE
 */
void terminate(int exitCode)
{
   glDeleteTextures(_posTexID.size(), &_posTexID[0]);
   glDeleteFramebuffers(_fboID.size(),     &_fboID[0]);
   glDeleteVertexArrays(1, &_computeVAO);
   glDeleteBuffers(1, &_computePosBuf);
   glDeleteBuffers(1, &_computeTexCoordBuf);
   glfwTerminate();
   exit(exitCode);
}

/**
 * Reload the shaders. If unsuccessful, print the errors to stderr and
 * leave the previous shader program in place
 */
void reloadShaders(void)
{
   try
   {
      _updateProg = unique_ptr<GL::Program>(new GL::Program(_updateVertFile, _updateFragFile));
      _renderProg = unique_ptr<GL::Program>(new GL::Program(_renderVertFile, _renderFragFile));
   }
   catch (std::runtime_error exception)
   {
      std::cerr << exception.what() << std::endl;
   }
}

/**
 * Check the status of an FBO
 */
void fboStatus(void)
{
   GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   std::string error;
   bool bufferComplete = false;
   switch(status)
   {
      case GL_FRAMEBUFFER_COMPLETE:
         error = "Framebuffer complete.";
         bufferComplete = true;
         break;
         
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
         error = "[ERROR] Framebuffer incomplete: Attachment is NOT complete.";
         bufferComplete = false;
         break;
         
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
         error = "[ERROR] Framebuffer incomplete: No image is attached to Framebuffer.";
         bufferComplete = false;
         break;
         
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
         error = "[ERROR] Framebuffer incomplete: Draw buffer.";
         bufferComplete = false;
         break;
         
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
         error = "[ERROR] Framebuffer incomplete: Read buffer.";
         bufferComplete = false;
         break;
         
      case GL_FRAMEBUFFER_UNSUPPORTED:
         error = "[ERROR] Unsupported by Framebuffer implementation.";
         bufferComplete = false;
         break;
         
      default:
         error = "[ERROR] Unknow error.";
         bufferComplete = false;
         break;
   }
   
   if(!bufferComplete)
   {
      throw std::runtime_error(error);
   }
}

/**
 * Create an FBO that has an RGBA 32-bit floating point texture
 * and a texture for holding depth values
 */
void createFBO()
{
   GL_ERR_CHECK();
   try
   {
      // Initialize the source and destination indices
      _srcCompute = 0;
      _dstCompute = 1;
      
      // Create OpenGL handles for texture and framebuffer objects
      _posTexID.resize(2);
      _velTexID.resize(2);
      _fboID.resize(2);
      
      glGenTextures(2, &_posTexID[0]);
      glGenTextures(2, &_velTexID[0]);
      glGenFramebuffers(2, &_fboID[0]);
      GL_ERR_CHECK();
      
      // Check for errors during texture and framebuffer generation
      bool fail = false;
      for(int i = 0; i < _posTexID.size(); ++i)
      {
         if(_posTexID[i] <= 0)
         {
            std::cerr << "Texture not generated for _posTexID[" << i << "]" << std::endl;
            fail = true;
         }

         if(_velTexID[i] <= 0)
         {
            std::cerr << "Texture not generated for _velTexID[" << i << "]" << std::endl;
            fail = true;
         }

         if(_fboID[i] <= 0)
         {
            std::cerr << "Framebuffer not generated for _fboID[" << i << "]"  << std::endl;
            fail = true;
         }
      }

      if(fail)
      {
         terminate(EXIT_FAILURE);
      }

      // Create the compute textures and attach them to the framebuffer object
      for(size_t id = 0; id < _posTexID.size(); ++id)
      {
         vec4* pos = &_positions[0];
         if(id == 1) pos = NULL;

         vec4* vel = &_velocities[0];
         if(id == 1) vel = NULL;

         // Create the texture map with the initial positions
         glBindTexture(GL_TEXTURE_2D, _posTexID[id]);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _particleWidth, _particleHeight, 0, GL_RGBA, GL_FLOAT, pos);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

         glBindTexture(GL_TEXTURE_2D, _velTexID[id]);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _particleWidth, _particleHeight, 0, GL_RGBA, GL_FLOAT, vel);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

         // Set up the framebuffer object (FBO)
         glBindFramebuffer(GL_FRAMEBUFFER, _fboID[id]);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _posTexID[id], 0);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _velTexID[id], 0);
         
         vector<GLuint> drawBuffers =
         {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
         };
         
         // Set the drawing buffer for this FBO
         glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);
         
         // Set the reading buffer for this FBO
         glReadBuffer(GL_COLOR_ATTACHMENT0);

         // Check the status
         fboStatus();
         
         // Unbind
         glBindTexture(GL_TEXTURE_2D, 0);
         glBindFramebuffer(GL_FRAMEBUFFER, 0);
         GL_ERR_CHECK();
      }
   }
   catch (std::runtime_error exception)
   {
      std::cerr << exception.what() << std::endl;
      terminate(EXIT_FAILURE);
   }
}

/**
 * Initialize a single particle
 *
 * @param i
 *    The index of the particle to initialize
 */
#if 0
void initParticle(int i)
{
   // Determine the initial velocity. The magnitude of all velocities is the
   // same and determined by the variable r, but the directions are random
   
   // Define the velocity in terms of polar coordinates
   float theta = 2 * M_PI * rand() / RAND_MAX;
   float phi = 2 * M_PI * rand() * RAND_MAX;
   float r = 3.51f;
   
   // Translate from polar to cartesian
   float sin_theta = sin(theta);
   float cos_theta = cos(theta);
   float sin_phi   = sin(phi);
   float cos_phi   = cos(phi);
   float x = r * cos_phi * sin_theta;
   float y = r * sin_phi * sin_theta;
   float z = r * cos_theta;
   
   _velocities.at(i) = vec4(x,y,z,0);
   
   float height = 0.1;
   // All particles start from a position directly above the gravity well
   if(i % 2 == 1)
   {
      height *= -1;
   }
   _positions.at(i)  = vec4(0, height, 0, 1.0f);
}
#else
/**
 * Initialize a single particle
 *
 * @param i
 *    The index of the particle to initialize
 */
void initParticle(int i)
{
   // Determine the initial velocity. The magnitude of all velocities is the
   // same and determined by the variable r, but the directions are random
   
   // Define the velocity in terms of polar coordinates
   float theta = 2 * M_PI * rand() / RAND_MAX;
   float phi = 2 * M_PI * rand() * RAND_MAX;
   float r = 3.51f;
   
   // Translate from polar to cartesian
   float sin_theta = sin(theta);
   float cos_theta = cos(theta);
   float sin_phi   = sin(phi);
   float cos_phi   = cos(phi);
   float x = r * cos_phi * sin_theta;
   float y = r * sin_phi * sin_theta;
   float z = r * cos_theta;
   
   _velocities.at(i) = vec4(x,y,z,0);
   
   // All particles start from a position directly above the gravity well
   _positions.at(i)  = vec4(0, 0.1, 0, 1.0f);
   
}
#endif


/**
 * Initialize particles
 */
void initParticles(size_t numParticles)
{
   // Set up a texture map that contains the positions
   _particleWidth = sqrtf(numParticles);
   _particleHeight = _particleWidth;
   _positions.resize(_particleHeight * _particleWidth);
   _velocities.resize(_particleHeight * _particleWidth);

   for(size_t i = 0; i < _particleWidth * _particleHeight; ++i)
   {
      initParticle(i);
   }
   
   createFBO();
   
   glGenVertexArrays(1, &_pVAO);
   glBindVertexArray(_pVAO);
   glGenBuffers(1, &_pBO);
   glBindBuffer(GL_ARRAY_BUFFER, _pBO);
   // GL_STATIC_DRAW has best performance. One would expect GL_STATIC_COPY would have the best performance,
   // but it is not the case.
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * _particleWidth * _particleWidth, NULL, GL_STATIC_DRAW);
   glVertexAttribPointer(_renderProg->getAttribLocation("pos"), 4, GL_FLOAT, GL_FALSE, 0, NULL);
   glEnableVertexAttribArray(_renderProg->getAttribLocation("pos"));
   glBindVertexArray(0);
}

/**
 * Initialize the computation quad. The quad covers the entire "screen"
 */
void initComputeQuad()
{
   glGenVertexArrays(1, &_computeVAO);
   glGenBuffers(1, &_computePosBuf);
   glGenBuffers(1, &_computeTexCoordBuf);
   
   vector<vec4> positions =
   {
      vec4(-1, -1, 0, 1),
      vec4(-1,  1, 0, 1),
      vec4( 1, -1, 0, 1),
      vec4( 1,  1, 0, 1),
   };
   
   float w = 1;
   vector<vec2> texcoords =
   {
      vec2(0, 0),
      vec2(0, w),
      vec2(w, 0),
      vec2(w, w)
   };
   
   glBindVertexArray(_computeVAO);

   glBindBuffer(GL_ARRAY_BUFFER, _computePosBuf);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * positions.size(), &positions[0], GL_STATIC_DRAW);
   glVertexAttribPointer(_updateProg->getAttribLocation("pos"), 4, GL_FLOAT, GL_FALSE, 0, NULL);
   glEnableVertexAttribArray(_updateProg->getAttribLocation("pos"));

   glBindBuffer(GL_ARRAY_BUFFER, _computeTexCoordBuf);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * texcoords.size(), &texcoords[0], GL_STATIC_DRAW);
   glVertexAttribPointer(_updateProg->getAttribLocation("texcoord"), 2, GL_FLOAT, GL_FALSE, 0, NULL);
   glEnableVertexAttribArray(_updateProg->getAttribLocation("texcoord"));

   glBindVertexArray(0);
}

/**
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
void init(size_t numParticles)
{
   // Set the location of the shader files
   _renderVertFile = std::string(SOURCE_DIR) + "/render_vert.c";
   _renderFragFile = std::string(SOURCE_DIR) + "/render_frag.c";
   _updateVertFile = std::string(SOURCE_DIR) + "/update_vert.c";
   _updateFragFile = std::string(SOURCE_DIR) + "/update_frag.c";
   
   // Load the shaders
   reloadShaders();
   
   // Verify that the shaders loaded. If they did not load,
   // then the program can't continue and must terminate
   if(_updateProg == nullptr || _renderProg == nullptr)
   {
      terminate(EXIT_FAILURE);
   }
   
   // Initial particle system, textures and FBOs
   initParticles(numParticles);
   initComputeQuad();
 
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);
   int width, height;
   glfwGetWindowSize(&width, &height);
}

/**
 * Window resize callback
 * 
 * @param width   the width of the window
 * @param height  the height of the window
 */
void GLFWCALL resize(int width, int height)
{
   // Set the affine transform of (x,y) from normalized device coordinates to
   // window coordinates. In this case, (-1,1) -> (0, width) and (-1,1) -> (0, height)
   glViewport(0, 0, width, height);
   
   _trackball->reshape(width, height);
}

/**
 *  Mouse click callback
 *
 *  @param button that was clicked
 *  @param button state
 */
void GLFWCALL mouseButton(int button, int action)
{
   if(button == GLFW_MOUSE_BUTTON_1)
   {
      if(action == GLFW_PRESS)
      {
         _tracking = true;
      }
      else
      {
         _tracking = false;
      }
   }
   
   
   if(_tracking)
   {
      int x, y;
      glfwGetMousePos(&x, &y);
      _trackball->start(x,y);
   }
   else
   {
      _trackball->stop();
   }
}

/**
 * Mouse movement callback
 */
void GLFWCALL mouseMove(int x, int y)
{
   if(_tracking)
   {
      int width, height;
      glfwGetWindowSize(&width, &height);
      _trackball->motion(x, height - y);
   }
}

/**
 * Handle changes in the mouse wheel position
 */
void GLFWCALL mouseWheel(int mouseWheelCur)
{
   // Find the change in the mouse wheel position
   float delta = _mouseWheelPrev - mouseWheelCur;
   
   // Update the zoom based on the delta
   _zoom += delta;
   
   // Keep the zoom amount in the range (_zoomMin,_zoomMax);
   if(_zoom > _zoomMax) {
      _zoom = _zoomMax;
   }
   
   if(_zoom < _zoomMin) {
      _zoom = _zoomMin;
   }
   
   // The new "previous" mouse wheel position
   _mouseWheelPrev = mouseWheelCur;
}

/**
 * Keypress callback
 */ 
void GLFWCALL keypress(int key, int state)
{
   if(state == GLFW_PRESS)
   {
      switch(key)
      {
         case GLFW_KEY_ESC:
            _running = false;
            break;
            
         case 'R':
         case 'r':
            reloadShaders();
            break;
            
      }
   }
}

/**
 * Window close callback
 */
int GLFWCALL close(void)
{
   _running = false;
   return GL_TRUE;
}

/**
 * Update particle positions
 */
void updateParticles()
{
   glDisable(GL_BLEND);
   // Set the viewport to be the size of the destination texture
   glViewport(0, 0, _particleWidth, _particleHeight);
   
   // Bind the framebuffer object
   glBindFramebuffer(GL_FRAMEBUFFER, _fboID[_dstCompute]);

   _updateProg->bind();
   
   // Bind the source particle attribute textures
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, _posTexID[_srcCompute]);
   _updateProg->setUniform("inPos", 0);
   
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, _velTexID[_srcCompute]);
   _updateProg->setUniform("inVel", 1);
   
   // Draw the quad that covers the entire FBO
   glBindVertexArray(_computeVAO);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   // Copy the positions from the FBO's back end buffer that hold the
   // particle positions into the vertex buffer object used to draw
   // the particles. It is hoped that a copy will not actually happen,
   // but instead that the driver will merely change a pointer.
   glReadBuffer(GL_COLOR_ATTACHMENT0);
   glBindBuffer(GL_PIXEL_PACK_BUFFER, _pBO);
   // Set the reading buffer for this FBO
   glReadPixels(0, 0, _particleWidth, _particleHeight, GL_RGBA, GL_FLOAT, NULL);
   glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
   GL_ERR_CHECK();

   // Reset the VAO, shader and framebuffer object state back to
   // the default
   glBindVertexArray(0);
   _updateProg->release();
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   
   // Reset the viewport size
   int width, height;
   glfwGetWindowSize(&width, &height);
   glViewport(0, 0, width, height);
   GL_ERR_CHECK();
   
   
   
   // Flip the source and destination texture / fbo indices
   _dstCompute ^= 1;
   _srcCompute ^= 1;
}

/**
 * Draw particle system
 */
void drawParticles()
{
   glEnable(GL_BLEND);
   // Clear the color and depth buffers
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glBindVertexArray(_pVAO);
   _renderProg->bind();
   
   glPointSize(1.0f);
   // Get the width and height of the window
   int width;
   int height;
   glfwGetWindowSize(&width, &height);
   
   // Clear the color and depth buffers
   GL_ERR_CHECK();
   
   // Projection matrix
   glm::mat4 projection = glm::perspective(45.0f,                         // 45 degree field of view
                                           float(width) / float(height),  // Ratio
                                           0.1f,                          // Near clip
                                           4000.0f);                      // Far clip
   // Camera matrix
   glm::mat4 view       = glm::lookAt(glm::vec3(0,0,_zoom * 1e-2), // Camera position is at (0,0,2), in world space
                                      glm::vec3(0,0,0), // and looks at the origin
                                      glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                      );
   
   glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
   
   
   // Model matrix
   glm::mat4 model = _trackball->getTransform();
   
   // Create  model, view, projection matrix
   glm::mat4 mv        = view * translate * model;
   
   
   _renderProg->setUniform("mv", mv);
   _renderProg->setUniform("proj", projection);
   glDrawArrays(GL_POINTS, 0, _particleWidth * _particleHeight);

   _renderProg->release();
   glBindVertexArray(0);
   GL_ERR_CHECK();
}

/**
 * Main loop
 * @param time    time elapsed in seconds since the start of the program
 */
int update(double time)
{
   try
   {
      updateParticles();
      drawParticles();
      _numFrames++;
   }
   catch (std::runtime_error exception)
   {
      std::cerr << exception.what() << std::endl;
   }
   return GL_TRUE;
}

/**
 * Calculate and display the frame rate for the entire run of the program
 */
void framerate(unsigned long frameCount)
{
   gettimeofday(&_endTime, NULL);
   double elapsed = _endTime.tv_sec - _startTime.tv_sec;
   std::cout << "Frames per second: " << frameCount / elapsed << std::endl;
}

/**
 * Program entry point
 */
int main(int argc, char* argv[])
{
   //   return 0;
   int width = 1280; // Initial window width
   int height = 720; // Initial window height
   _running = true;
   _numFrames = 0;
   _zoomMax = 6000;
   _zoomMin = 1;
   _zoom = 700;
   _tracking = false;
   
   _trackball = unique_ptr<Trackball>(new Trackball(width, height));

   // Initialize GLFW
   glfwInit();

   // Request an OpenGL core profile context, without backwards compatibility
   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR,  3);
   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR,  2);
   glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwOpenWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
   // Open a window and create its OpenGL context
   if(!glfwOpenWindow(width, height, 0, 0, 0, 8, 32, 0, GLFW_WINDOW ))
   {
      std::cerr << "Failed to open GLFW window" << std::endl;
      glfwTerminate();
      return -1;
   }
   resize(width, height);

   glfwSwapInterval(0);
   glfwSetWindowSizeCallback(resize);
   glfwSetKeyCallback(keypress);
   glfwSetWindowCloseCallback(close);
   glfwSetMouseButtonCallback(mouseButton);
   glfwSetMouseWheelCallback(mouseWheel);
   glfwSetMousePosCallback(mouseMove);
   
   std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;

#if 0
   vector<float> numParticles =
   {
      250000,
      500000,
      750000,
      1000000,
      2000000,
      3000000,
      4000000,
      5000000,
      6000000,
      7000000,
      8000000,
      9000000,
      10000000,
      11000000,
      12000000,
      13000000,
      14000000,
      15000000,
   };
   
   for(auto num : numParticles)
   {
      init(num);
      _numFrames = 0;
      gettimeofday(&_startTime, NULL);
      
      // Main loop. Run until ESC key is pressed or the window is closed
      while(_running && _numFrames < 1000)
      {
         update(glfwGetTime());
         glfwSwapBuffers();
      }
      std::cout << "num: " << num << std::endl;
      framerate(_numFrames);
      if(!_running)
      {
         break;
      }
   }
   
#else
   size_t num = 1000000;
   init(num);
   _numFrames = 0;
   gettimeofday(&_startTime, NULL);
   
   // Main loop. Run until ESC key is pressed or the window is closed
   while(_running)
   {
      update(glfwGetTime());
      glfwSwapBuffers();
   }
   std::cout << "num: " << num << std::endl;
   framerate(_numFrames);
#endif
   
   terminate(EXIT_SUCCESS);
}
