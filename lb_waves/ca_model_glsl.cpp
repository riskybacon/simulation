//--------------------------------------------------------------------------------
// ca_model_glsl.cpp
//
// Cellular Automata GLSL Model front-end. Separates out the model of the CA from
// the view of the CA. The idea is that this model could be replaced with a CPU
// based model, a CUDA based model, or an OpenCL based model
//
// CS 523 Spring 2013
// Project 3
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------
#include <iostream>
#include <stdexcept>

#include "ca_model_glsl.h"

using glm::vec2;
using glm::vec4;
using std::vector;


/**
 * Constructor. Initializes the cells in the model
 *
 * @param   size
 *    The lattice size
 * @param   min
 *    The (x,y) position at lattice position (0,0)
 * @param   max
 *    The (x,y) position at lattice position (size.x, size.y)
 * @param   physicalSize
 *    The physical dimensions in meters for on side of the simulation. Assume simulation is square
 * @param   timeStep
 *    The amount of time to step the simulation in seconds
 * @param   computeProg
 *    The GLSL program that performs the update step
 */
CAModelGLSL::CAModelGLSL(const glm::ivec2& size, const glm::vec2& min, const glm::vec2& max, float physicalSize, float timeStep, GL::Program* computeProg)
: _size        (size)
, _min         (min)
, _max         (max)
, _src         (0)
, _dst         (1)
, _computeProg (computeProg)
, _physicalSize(physicalSize)
, _timeStep    (timeStep)
, _ocean       (Ocean(size.x, 0.00005f, vec2(0.0f,32.0f), 64))

{
   // Initial state
   initialStateGaussianAndPhillips();
   // Create computation data structures on the GPU
   createFBO();
   // Set up the computation quad
   initComputeQuad();
}

/*
 * Destructor
 */
CAModelGLSL::~CAModelGLSL()
{
   delete _computeProg;
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

/*
 * Set the initial state of the CA
 */
void CAModelGLSL::initialStateGaussian()
{
   _positions.clear();
   _massFlow0.clear();
   _massFlow1.clear();
   _positions.reserve(_size.x * _size.y);
   _massFlow0.reserve(_size.x * _size.y);
   _massFlow1.reserve(_size.x * _size.y);
   
   _step.x = (_max.x - _min.x) / (_size.x - 1.0);
   _step.y = (_max.y - _min.y) / (_size.y - 1.0);

   // Assuming _size.x == _size.y
   _lambda = _physicalSize / _size.x;
   
   // Number of times this wave occurs over the surface
   _waveNumber = 1;
   float A = 4;
   
   float simWidth = _max.x - _min.x;
   float simHeight = _max.y - _min.y;
   
   vec2 pos(0, _min.y);
   for(int y = 0; y < _size.y; y++, pos.y += _step.y)
   {
      pos.x = _min.x;
      for(int x = 0; x < _size.x; x++, pos.x += _step.x)
      {
         std::vector<vec2> pos0;
         pos0.resize(4);
         
         pos0[0] = pos + 0.25f * vec2(simWidth,  simHeight);
         pos0[1] = pos - 0.25f * vec2(simWidth,  simHeight);
         pos0[2] = pos + 0.25f * vec2(simWidth, -simHeight);
         pos0[3] = pos - 0.25f * vec2(simWidth, -simHeight);
         
         
         float height = 0;
         for(int i = 0; i < pos0.size(); i++)
         {
            float length = glm::length(pos0[i]) / (_max.x - _min.x);
            height += A * expf(-50 * length * length);
         }

         glm::vec4 pos4(pos.x, height, pos.y, 1.0);
         _positions.push_back(pos4);
         
         float flow = height / 5.0;
         _massFlow0.push_back(vec4(flow, flow, flow, flow));
         _massFlow1.push_back(vec4(flow, _waveNumber, 0, 0));
      }
   }

}

/*
 * Set the initial state of the CA
 */
void CAModelGLSL::initialStateGaussianAndPhillips()
{
   _ocean.evaluateWavesFFT(1.0f / 30.0f);
   
   _positions.clear();
   _massFlow0.clear();
   _massFlow1.clear();
   _positions.reserve(_size.x * _size.y);
   _massFlow0.reserve(_size.x * _size.y);
   _massFlow1.reserve(_size.x * _size.y);
   
   _step.x = (_max.x - _min.x) / (_size.x - 1.0);
   _step.y = (_max.y - _min.y) / (_size.y - 1.0);
   
   // Assuming _size.x == _size.y
   _lambda = _physicalSize / _size.x;
   
   // Number of times this wave occurs over the surface
   _waveNumber = 1;
   float A = 4;
   
   float simWidth = _max.x - _min.x;
   float simHeight = _max.y - _min.y;
   
   int idx = 0;
   const std::vector<glm::vec4>& vertices = _ocean.getVertices();
   
   vec2 pos(0, _min.y);
   for(int y = 0; y < _size.y; y++, pos.y += _step.y)
   {
      pos.x = _min.x;
      for(int x = 0; x < _size.x; x++, pos.x += _step.x)
      {
         std::vector<vec2> pos0;
         pos0.resize(4);
         
         pos0[0] = pos + 0.25f * vec2(simWidth,  simHeight);
         pos0[1] = pos - 0.25f * vec2(simWidth,  simHeight);
         pos0[2] = pos + 0.25f * vec2(simWidth, -simHeight);
         pos0[3] = pos - 0.25f * vec2(simWidth, -simHeight);
         
         
         float height = 0;
         for(int i = 0; i < pos0.size(); i++)
         {
            float length = glm::length(pos0[i]) / (_max.x - _min.x);
            height += A * expf(-50 * length * length);
         }
         
         height += vertices[idx].y;
         
         glm::vec4 pos4(pos.x, height, pos.y, 1.0);
         _positions.push_back(pos4);
         
         float flow = height / 5.0;
         _massFlow0.push_back(vec4(flow, flow, flow, flow));
         _massFlow1.push_back(vec4(flow, _waveNumber, 0, 0));
         idx++;
      }
   }
   
}

/*
 * Set the initial state of the CA
 */
void CAModelGLSL::initialStatePhillips()
{
   _ocean.evaluateWavesFFT(1.0f / 30.0f);
   
   _positions.clear();
   _massFlow0.clear();
   _massFlow1.clear();
   _positions.reserve(_size.x * _size.y);
   _massFlow0.reserve(_size.x * _size.y);
   _massFlow1.reserve(_size.x * _size.y);
   
   _step.x = (_max.x - _min.x) / (_size.x - 1.0);
   _step.y = (_max.y - _min.y) / (_size.y - 1.0);
   
   // Assuming _size.x == _size.y
   _lambda = _physicalSize / _size.x;
   
   // Number of times this wave occurs over the surface
   _waveNumber = 1;
   
   const std::vector<glm::vec4>& vertices = _ocean.getVertices();

   vec2 pos(0, _min.y);
   int idx = 0;
   for(int y = 0; y < _size.y; y++, pos.y += _step.y)
   {
      pos.x = _min.x;
      for(int x = 0; x < _size.x; x++, pos.x += _step.x)
      {
         glm::vec4 pos4(pos.x, vertices[idx].y, pos.y, 1.0);
         _positions.push_back(pos4);
         
         float flow = vertices[idx].y / 5.0;
         _massFlow0.push_back(vec4(flow, flow, flow, flow));
         _massFlow1.push_back(vec4(flow, _waveNumber, 0, 0));
         idx++;
      }
   }
}

/*
 * Upload initial conditions to the GPU. This copies the data in
 * _positions, _massFlow0 and _massFlow1 to the source and destinatibon
 * texture maps
 */
void CAModelGLSL::uploadInitialConditions()
{
   // Upload initial conditions for each texture map
   for(unsigned int id = 0; id < _posTexID.size(); ++id)
   {
      // Create the texture map with the initial positions
      glBindTexture(GL_TEXTURE_2D, _posTexID[id]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getLatticeSize().x, getLatticeSize().y, 0, GL_RGBA, GL_FLOAT, &_positions[0]);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      
      // Create the texture map with the initial mass flow
      glBindTexture(GL_TEXTURE_2D, _massFlowTexID0[id]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getLatticeSize().x, getLatticeSize().y, 0, GL_RGBA, GL_FLOAT, &_massFlow0[0]);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      
      // Create the texture map with the initial mass flow
      glBindTexture(GL_TEXTURE_2D, _massFlowTexID1[id]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getLatticeSize().x, getLatticeSize().y, 0, GL_RGBA, GL_FLOAT, &_massFlow1[0]);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   }

}

/*
 * Create a framebuffer object to hold results of GPU computation
 */
void CAModelGLSL::createFBO()
{
   GL_ERR_CHECK();
   try
   {
      _stepT = 1.0f / _size.y;
      _stepS = 1.0f / _size.x;

      // Create OpenGL handles for texture and framebuffer objects
      _posTexID.resize(2);
      _massFlowTexID0.resize(2);
      _massFlowTexID1.resize(2);
      
      _fboID.resize(2);
      
      // Generate texture handles
      glGenTextures(_posTexID.size(), &_posTexID[0]);
      glGenTextures(_massFlowTexID0.size(), &_massFlowTexID0[0]);
      glGenTextures(_massFlowTexID1.size(), &_massFlowTexID1[0]);
      // Generate framebuffer handles
      glGenFramebuffers(_fboID.size(), &_fboID[0]);
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
         if(_massFlowTexID0[i] <= 0)
         {
            std::cerr << "Texture not generated for _massFlowTexID[" << i << "]" << std::endl;
            fail = true;
         }
         
         if(_fboID[i] <= 0)
         {
            std::cerr << "Framebuffer not generated for _fboID[" << i << "]"  << std::endl;
            fail = true;
         }
      }
      
      // This sucks - should probably just throw an error, but this is research code
      assert(fail == false);

      // Upload the initial conditions
      uploadInitialConditions();
      
      // Bind the textures to the FBOs
      for(unsigned int id = 0; id < _posTexID.size(); ++id)
      {
         // Set up the framebuffer object (FBO)
         glBindFramebuffer(GL_FRAMEBUFFER, _fboID[id]);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _posTexID[id], 0);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _massFlowTexID0[id], 0);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _massFlowTexID1[id], 0);
         
         vector<GLuint> drawBuffers =
         {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
         };
         
         // Set the drawing buffer for this FBO
         glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);
         
         // Set the reading buffer for this FBO
         glReadBuffer(GL_NONE);
         
         // Check the status
         fboStatus();
      }
      
      // Unbind texture and FBO
      glBindTexture(GL_TEXTURE_2D, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      GL_ERR_CHECK();
   }
   catch (std::runtime_error exception)
   {
      std::cerr << exception.what() << std::endl;
      exit(0);
   }
}

/*
 * Initialize the computation quad. The quad covers the entire "screen"
 */
void CAModelGLSL::initComputeQuad()
{
   glGenVertexArrays(1, &_computeVAO);
   glGenBuffers(1, &_computeBuf);
   
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
   
   // Calculate the size of the buffer
   size_t posSize = positions.size() * sizeof(vec4);
   size_t tcSize  = texcoords.size() * sizeof(vec2);
   size_t bufsize = posSize + tcSize;
   
   GLuint posAttr = _computeProg->getAttribLocation("pos");
   GLuint tcAttr  = _computeProg->getAttribLocation("texcoord");
   
   glBindBuffer(GL_ARRAY_BUFFER, _computeBuf);
   // Ask the GL to allocate a buffer that can hold the positions and texcoords
   glBufferData(GL_ARRAY_BUFFER, bufsize, NULL, GL_STATIC_DRAW);
   // Copy in the position data starting at the beginning of the buffer
   glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, &positions[0]);
   // Copy in the texcoord data starting at the posSize offset in the buffer
   glBufferSubData(GL_ARRAY_BUFFER, posSize, tcSize, &texcoords[0]);
   
   // The pos attribute gets its data from the start of the buffer
   glVertexAttribPointer(posAttr, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
   glEnableVertexAttribArray(posAttr);
   
   // The texcoord attribute gets its data starting at the posSize offset in the buffer
   glVertexAttribPointer(tcAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)posSize);
   glEnableVertexAttribArray(tcAttr);
   
   glBindVertexArray(0);
}

/*
 * Update the model
 */
void CAModelGLSL::update()
{
   try
   {
      GL_ERR_CHECK();
      // Flip the source and destination texture / fbo indices
      _dst ^= 1;
      _src ^= 1;
      
      glDisable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);
      glClearColor(0, 0, 0, 0);
      // Set the viewport to be the size of the destination texture
      glViewport(0, 0, _size.x, _size.y);
      
      GL_ERR_CHECK();
      // Bind the framebuffer object
      glBindFramebuffer(GL_FRAMEBUFFER, _fboID[_dst]);
      GL_ERR_CHECK();
      glClear(GL_COLOR_BUFFER_BIT);
      GL_ERR_CHECK();
      _computeProg->bind();
      GL_ERR_CHECK();
      
      // Bind the source particle attribute textures
      glActiveTexture(GL_TEXTURE0);
      GL_ERR_CHECK();
      glBindTexture(GL_TEXTURE_2D, _posTexID[_src]);
      GL_ERR_CHECK();
      _computeProg->setUniform("pos", 0);
      GL_ERR_CHECK();
      
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _massFlowTexID0[_src]);
      _computeProg->setUniform("massFlow0", 1);
      GL_ERR_CHECK();
      
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, _massFlowTexID1[_src]);
      _computeProg->setUniform("massFlow1", 2);
      GL_ERR_CHECK();
      
      _computeProg->setUniform("stepS",    _stepS);
      _computeProg->setUniform("stepT",    _stepT);
      _computeProg->setUniform("lambda",   _lambda);
      _computeProg->setUniform("timeStep", _timeStep);

      // Draw the quad that covers the entire FBO
      glBindVertexArray(_computeVAO);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      
      // Reset the VAO, shader and framebuffer object state back to
      // the default
      glBindVertexArray(0);
      _computeProg->release();
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }
   catch(std::runtime_error err)
   {
      std::cerr << "CAModelGLSL::update execption caught:" << err.what() << std::endl;
   }
   
}
