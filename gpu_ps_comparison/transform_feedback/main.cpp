#include <iostream>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/virtrev/xstream.hpp>       // ostream
#include <glm/gtc/matrix_transform.hpp>  // rotation, translation, scale, etc
#include <glm//gtc/type_ptr.hpp>         // value_ptr
#include <memory>

#include "opengl.h"
#include "shader.h"

#include <vector>
#include <string>
#include <sys/time.h>

using std::vector;
using std::string;
using std::auto_ptr;
using namespace glm;

auto_ptr<GL::Program> _updateProg;
auto_ptr<GL::Program> _renderProg;

string         _updateVertFile;
string         _renderVertFile;
string         _renderFragFile;

bool           _running = true;

vector<GLuint> _pVao;            //< Vertex array object for the positions
vector<GLuint> _pVbo;            //< buffer object for the the positions;
vector<GLuint> _vVbo;            //< Buffer object for the velocities
vector<GLuint> _aVbo;            //< Buffer object for the acceleration
vector<GLuint> _pInitBO;         //< buffer object for the the initial positions;
vector<GLuint> _vInitBO;         //< Buffer object for the initial velocities

vector<string> _varyings;

glm::quat      _objQuat;         //< Quaternion that represents the rotation of the object
glm::mat4      _objMat;          //< Matrix that represents the rotation of the object

unsigned int   _source = 0;      //< Ping pong buffer
unsigned int   _dest   = 1;
GLuint         _query;           //< Handle to a query object

vector<vec4>   _positions;
vector<vec4>   _velocities;

int            _width;
int            _height;

timeval        _startTime;
timeval        _endTime;

unsigned long  _numFrames;

bool           _tracking;

// This keeps the user from zooming really far in or out and then having to spend
// time to undo their actions. Zooming is clamped to a max and min value by
// checking the delta of the mouse wheel each time it changes.
float         _mouseWheelPrev;  //< Previous mouse wheel position;
float         _zoom;            //< Current mouse wheel zoom in/out factor
float         _zoomMax;         //< Max allowed mouse wheel zoom
float         _zoomMin;         //< Min allowed mouse wheel zoom

int           _startX;
int           _startY;

bool           _dirty;

/**
 * Clean up and exit
 *
 * @param exitCode      The exit code, eg, EXIT_SUCCESS or EXIT_FAILURE
 */
void terminate(int exitCode)
{
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
      
      auto_ptr<GL::Program> newProgram;
      
      newProgram = auto_ptr<GL::Program>(new GL::Program(_updateVertFile, _varyings, GL_SEPARATE_ATTRIBS));
      GL_ERR_CHECK();
      _updateProg = newProgram;

      newProgram = auto_ptr<GL::Program>(new GL::Program(_renderVertFile, _renderFragFile));
      GL_ERR_CHECK();
      _renderProg = newProgram;
   }
   catch (const std::runtime_error& err)
   {
      std::cerr << err.what() << std::endl;
      throw err;
   }
}

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

/**
 * Initialize all of the particles
 */
void initParticles(int numParticles)
{
   _velocities.resize(numParticles);
   _positions.resize(numParticles);
   
   for(size_t i = 0; i < numParticles; ++i)
   {
      initParticle(i);
   }
}

/**
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
void init(int numParticles)
{
   try
   {
      _renderVertFile = std::string(SOURCE_DIR) + "/ps_vert.c";
      _renderFragFile = std::string(SOURCE_DIR) + "/ps_frag.c";
      _updateVertFile = std::string(SOURCE_DIR) + "/update_vert.c";
      
      
      _varyings = {"newVel", "newPos"};

      // Load the shaders. Exit if one or more did not load
      reloadShaders();


      initParticles(numParticles);
      
      _pVao.resize(2);
      _pVbo.resize(2);
      _vVbo.resize(2);
                      
      glGenVertexArrays(2, &_pVao[0]);
      glGenBuffers(2, &_pVbo[0]);
      glGenBuffers(2, &_vVbo[0]);
      
      for(size_t buf = 0; buf < _pVao.size(); ++buf)
      {
         glBindVertexArray(_pVao[buf]);

         glBindBuffer(GL_ARRAY_BUFFER, _pVbo[buf]);
         glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * _positions.size(), &_positions[0], GL_STREAM_COPY);
         glVertexAttribPointer(_updateProg->getAttribLocation("pos"), 4, GL_FLOAT, GL_FALSE, 0, NULL);
         glEnableVertexAttribArray(_updateProg->getAttribLocation("pos"));

         glBindBuffer(GL_ARRAY_BUFFER, _vVbo[buf]);
         glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * _velocities.size(), &_velocities[0], GL_STREAM_COPY);
         glVertexAttribPointer(_updateProg->getAttribLocation("vel"), 4, GL_FLOAT, GL_FALSE, 0, NULL);
         glEnableVertexAttribArray(_updateProg->getAttribLocation("vel"));
      }
      
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE);

   }
   catch (std::runtime_error exception)
   {
      std::cerr << exception.what() << std::endl;
      terminate(EXIT_FAILURE);
   }
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
 * Create a quaternion using a rotation about an axis
 */
glm::quat createQuat(float angle, const glm::vec3& axis)
{
   glm::quat quat;
   angle *= 0.5f;
   float sinAngle;
   
   sinAngle = sin(angle);
   
   quat.x = (axis.x * sinAngle);
   quat.y = (axis.y * sinAngle);
   quat.z = (axis.z * sinAngle);
   quat.w = cos(angle);
   
   return quat;
}

// Rotation
void panGesture(float x, float y)
{
   const float sensitivity = 0.005; // * M_PI / 180.0f;
   
   const glm::vec3 yAxis(0, 1, 0);
   const glm::vec3 xAxis(1, 0, 0);
   
   // This operation looks backwards, but movement in the x direction on
   // the touch screen rotates the model about the y-axis
   glm::quat yRot = createQuat(x * sensitivity, yAxis);
   glm::quat xRot = createQuat(y * sensitivity, xAxis);
   
   _objQuat = glm::normalize(yRot * xRot * _objQuat);
   _objMat = glm::mat4_cast(_objQuat);
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
      glfwGetMousePos(&_startX, &_startY);
      _dirty = true;
      
   }
}

/**
 * Mouse movement callback
 */
void GLFWCALL mouseMove(int x, int y)
{
   if(_tracking)
   {
      int deltaX = x - _startX;
      int deltaY = y - _startY;
      
      panGesture(deltaX, deltaY);
      
      _startX = x;
      _startY = y;
      _dirty = true;
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

void drawPoints()
{
   glBindVertexArray(_pVao[_source]);
   glDrawArrays(GL_POINTS, 0, _positions.size());
   glBindVertexArray(0);
}

void transformFeedback()
{
   // Update happens in a vertex shader. There are two outputs from the vertex shader:
   // position and velocity
   try
   {
      _updateProg->bind();
      glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, _updateProg->getVaryingLocation("newPos"), _pVbo[_dest]);
      glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, _updateProg->getVaryingLocation("newVel"), _vVbo[_dest]);
      glBeginTransformFeedback(GL_POINTS);
      glEnable(GL_RASTERIZER_DISCARD);
      drawPoints();
      glDisable(GL_RASTERIZER_DISCARD);
      glEndTransformFeedback();
      _updateProg->release();
   }
   catch(std::runtime_error err)
   {
      std::cerr << err.what() << std::endl;
   }

   // Flip the source and destination buffers
   _dest ^= 1;
   _source ^= 1;
}

void draw(double time)
{
   try
   {
      glPointSize(1.0f);
      GL_ERR_CHECK();
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
      
      
      // Create  model, view, projection matrix
      glm::mat4 mv        = view * translate * _objMat;

      
      _renderProg->bind();
      _renderProg->setUniform("mv", mv);
      _renderProg->setUniform("proj", projection);
      drawPoints();
      GL_ERR_CHECK();
      _renderProg->release();

      
   }
   catch (std::runtime_error exception)
   {
      std::cerr << exception.what() << std::endl;
   }
}

/**
 * Main loop
 * @param time    time elapsed in seconds since the start of the program
 */
int update(double time)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   transformFeedback();
   draw(time);
   ++_numFrames;
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
 * Window resize callback
 *
 * @param width   the width of the window
 * @param height  the height of the window
 */
void GLFWCALL resize(int width, int height)
{
   // Set the affine transform of (x,y) from normalized device coordinates to
   // window coordinates. In this case, (-1,1) -> (0, width) and (-1,1) -> (0, height)
   _width = width;
   _height = height;
   glViewport(0, 0, _width, _height);
}

/**
 * Program entry point
 */
int main(int argc, char* argv[])
{
   //   return 0;
   _width = 1024; // Initial window width
   _height = 768; // Initial window height
   _running = true;
   _tracking = false;
   _numFrames = 0;
   
   _zoomMax = 6000;
   _zoomMin = 1;
   _zoom = 700;

   // Initialize GLFW
   glfwInit();

   // Request an OpenGL core profile context, without backwards compatibility
   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR,  3);
   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR,  2);
   glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwOpenWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
   glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 8);
   //   glfwSwapInterval(0);
   // Open a window and create its OpenGL context
   if(!glfwOpenWindow(_width, _height, 0, 0, 0, 8, 32, 0, GLFW_WINDOW))
   {
      std::cerr << "Failed to open GLFW window" << std::endl;
      glfwTerminate();
      return -1;
   }

   resize(_width, _height);

   glfwSwapInterval(0);

   glfwSetWindowSizeCallback(resize);
   glfwSetKeyCallback(keypress);
   glfwSetWindowCloseCallback(close);
   glfwSetMouseButtonCallback(mouseButton);
   glfwSetMousePosCallback(mouseMove);
   glfwSetMouseWheelCallback(mouseWheel);
   
   std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;

   
#if 0
   // This section used to test FPS with varying numbers of particles
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

   init(250000);

   gettimeofday(&_startTime, NULL);

   // Main loop. Run until ESC key is pressed or the window is closed
   while(_running)
   {
      update(glfwGetTime());
      glfwSwapBuffers();
   }
   framerate(_numFrames);

   terminate(EXIT_SUCCESS);
#endif
}
