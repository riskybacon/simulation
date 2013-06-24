//--------------------------------------------------------------------------------
// main.cpp
//
// Entry point to the program. Sets up the OpenGL context, the Scene object and
// the GLFW callbacks. The callbacks are routed to the Scene object. This also
// has the ability to take screenshots
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------

#include <GL/glfw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "scene.h"

bool           _running;                  //< true if the program is running, false if it is time to terminate

Scene*         _scene;

// Size of window
int            _winWidth;
int            _winHeight;
bool           _tracking;

int            _startX;
int            _startY;

int            _mouseWheelPrev;
bool           _dirty;
bool           _paused;

int            _frame;

timeval        _startTime;
timeval        _endTime;

#include <sys/time.h>

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

//----------------------------------------------------------------------
// Write frame to file
//----------------------------------------------------------------------
void saveFrame(void)
{
   char filename[1024];
   FILE* f;
   int size, n;
   
   std::string prefix = std::string(SOURCE_DIR) + "/frame";
   
   snprintf(filename, sizeof(filename), "%s-%06d.ppm", prefix.c_str(), _frame);
   f = fopen(filename, "wb");
   if(f == NULL)
   {
      std::cerr << "Unable to open " << filename << std::endl;
      return;
   }
   
   size = _winWidth * _winHeight * 3;
   GLvoid* pixels = malloc(size);
   
   glReadBuffer(GL_FRONT);
   /* Can't assume aligned on 32 bit boundary */
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glReadPixels(0, 0, _winWidth, _winHeight,
                GL_RGB, GL_UNSIGNED_BYTE, pixels);
   
   fprintf(f, "P6 %d %d 255\n", _winWidth, _winHeight);
   n = fwrite(pixels, 1, size, f);
   if(n != size)
   {
      std::cerr << "Error writing frame " << _frame << std::endl;
      return;
   }
   fclose(f);
   
   free(pixels);
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
   _scene->resize(width, height);
   _winHeight = height;
   _winWidth = width;
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
            _scene->loadShaders();
            break;
            
         case 'G':
         case 'g':
            _scene->resetToInitialConditionsGaussian();
            break;

         case 'P':
         case 'p':
            _scene->resetToInitialConditionsPhillips();
            break;

         case 'B':
         case 'b':
            _scene->resetToInitialConditionsGaussianAndPhillips();
            break;
            
         case 'S':
         case 's':
            saveFrame();
            break;
            
         case 'C':
         case 'c':
            _scene->resetCameraToInitialState();
            break;
            
         case GLFW_KEY_SPACE:
            _paused = !_paused;
            if(_paused)
            {
               std::cout << "paused" << std::endl;
            }
            else
            {
               std::cout << "running" << std::endl;
            }
            break;
      }
   }
   _dirty = true;
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
 * Main loop
 * @param time    time elapsed in seconds since the start of the program
 */
int update(double time)
{
   try
   {
      if(!_paused)
      {
         _scene->update();
         _dirty = true;
      }

      if(_dirty)
      {
         _scene->draw();
         _dirty = false;
         glfwSwapBuffers();
      }
      else
      {
         glfwWaitEvents();
      }
   }
   catch (std::runtime_error exception)
   {
      std::cerr << exception.what() << std::endl;
   }
   
   return GL_TRUE;
   return GL_TRUE;
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
      
      _scene->panGesture(deltaX, deltaY);
      
      _startX = x;
      _startY = y;
      _dirty = true;
   }
}

/**
 * Handle changes in the mouse wheel position
 */
void GLFWCALL mouseWheel(int mouseWheelCur)
{
   // Find the change in the mouse wheel position
   float delta = _mouseWheelPrev - mouseWheelCur;
   
   _scene->pinchGesture(delta, 0.25);
   
   // The new "previous" mouse wheel position
   _mouseWheelPrev = mouseWheelCur;
   _dirty = true;
}

/**
 * Calculate and display the frame rate for the entire run of the program
 */
void framerate(void)
{
   gettimeofday(&_endTime, NULL);
   double elapsed = _endTime.tv_sec - _startTime.tv_sec;
   std::cout << "Frame per second: " << _frame / elapsed << std::endl;
}

/**
 * Program entry point
 */
int main(int argc, char* argv[])
{
   int _winWidth = 1280;
   int _winHeight = 720;
   
   _frame = 0;
   _running = true;
   _tracking = false;
   _paused = false;
   _dirty = true;
   
   // Initialize GLFW
   glfwInit();

   // Request an OpenGL core profile context, without backwards compatibility
   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR,  3);
   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR,  2);
   glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwOpenWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
   glfwOpenWindowHint(GLFW_FSAA_SAMPLES,          8);

   // Open a window and create its OpenGL context
   if(!glfwOpenWindow(_winWidth, _winHeight, 0, 0, 0, 0, 32, 0, GLFW_WINDOW ))
   {
      std::cerr << "Failed to open GLFW window" << std::endl;
      glfwTerminate();
      return -1;
   }

   _scene = new Scene(std::string(SOURCE_DIR), _winWidth, _winHeight);

   // Uncomment this line to test frame rate
   //glfwSwapInterval(0);
   glfwSetWindowSizeCallback(resize);
   glfwSetKeyCallback(keypress);
   glfwSetWindowCloseCallback(close);
   glfwSetMouseButtonCallback(mouseButton);
   glfwSetMouseWheelCallback(mouseWheel);
   glfwSetMousePosCallback(mouseMove);

   std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;

   _mouseWheelPrev = glfwGetMouseWheel();

   // Get the starting time
   gettimeofday(&_startTime, NULL);

   // Main loop. Run until ESC key is pressed or the window is closed
   while(_running)
   {
      update(glfwGetTime());
      //      saveFrame();
      _frame++;
   }
   framerate();
   
   
   terminate(EXIT_SUCCESS);
}
