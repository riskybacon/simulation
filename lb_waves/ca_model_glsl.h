//--------------------------------------------------------------------------------
// ca_model_glsl.h
//
// Cellular Automata GLSL Model front-end. Separates out the model of the CA from
// the view of the CA. The idea is that this model could be replaced with a CPU
// based model, a CUDA based model, or an OpenCL based model.
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------
#ifndef _ca_model_glsl_h
#define _ca_model_glsl_h

#include <glm/glm.hpp>
#include <vector>
#include <iostream>

#include "shader.h"
#include "ocean.h"

/**
 * The cellular automata model for the waves
 */
class CAModelGLSL
{
public:
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
   CAModelGLSL(const glm::ivec2& size, const glm::vec2& min, const glm::vec2& max, float physicalSize, float timeStep, GL::Program* prog);
   
   /**
    * Destructor
    */
   ~CAModelGLSL();
   
   /**
    * Update the model to the next time step
    */
   void update();
   
   /**
    * Get the current position texture ID
    */
   const GLuint getCurPositionID() const
   {
      return _posTexID[_dst];
   }

   /**
    * @return the lattice size
    */
   const glm::ivec2 getLatticeSize() const
   {
      return _size;
   }
   
   /**
    * Set the compute shader
    */
   void setProgram(GL::Program* prog)
   {
      _computeProg = prog;
   }

   /**
    * Set the initial state of the CA using 4 equally spaced gaussians
    */
   void initialStateGaussian();

   /**
    * Set the initial state of the CA using the phillips spectrum
    */
   void initialStatePhillips();

   
   /**
    * Set the initial state of the CA using both phillips and gaussian
    */
   void initialStateGaussianAndPhillips();

   /**
    * Upload initial conditions to the GPU. This copies the data in
    * _positions, _massFlow0 and _massFlow1 to the source and destination
    * texture maps
    */
   void uploadInitialConditions();

protected:
   /**
    * Create a framebuffer object to hold results of GPU computation
    */
   void createFBO();

   /**
    * Initialize the computation quad. The quad covers the entire "screen"
    */
   void initComputeQuad();
   
private:
   glm::ivec2                    _size;               //< Lattice size
   glm::vec2                     _min;                //< The (x,y) position at lattice position (0,0)
   glm::vec2                     _max;                //< The (x,y) position at lattice position (_size.x, _size.y)
   unsigned int                  _src;                //< Current time step compute texture
   unsigned int                  _dst;                //< Destination compute texture
   std::vector<GLuint>           _posTexID;           //< Texture IDs for the position textures
   std::vector<GLuint>           _massFlowTexID0;     //< Texture IDs for the mass flow textures
   std::vector<GLuint>           _massFlowTexID1;     //< Texture IDs for the mass flow textures
   std::vector<GLuint>           _velTexID0;          //< Texture IDs for the velocities
   std::vector<GLuint>           _velTexID1;          //< Texture IDs for the velocities
   std::vector<GLuint>           _fboID;              //< Frame buffer object handles
   GLuint                        _computeVAO;         //< Vertex array object for the compute quad
   GLuint                        _computeBuf;         //< Buffer object for compute quad positions
   glm::vec2                     _step;               //< The (x,y) spacing between each lattice position
   GL::Program*                  _computeProg;        //< Pointer to the GLSL computation program
   std::vector<glm::vec4>        _positions;          //< Positions of the cells in the CA
   std::vector<glm::vec4>        _massFlow0;          //< Mass flow at each position c0 thru c3
   std::vector<glm::vec4>        _massFlow1;          //< Mass flow at each position c4
   float                         _stepS;              //< Distance in S to the next texel
   float                         _stepT;              //< Distance in T to the next texel
   int                           _waveNumber;         //< Number of times the wave occurs over the lattice
   float                         _physicalSize;       //< Physical size of the simulation in meters
   float                         _timeStep;           //< Amount of time to step the simulation in seconds;
   float                         _lambda;             //< spacing between lattice points, in meters
   glm::vec2                     _v;                  //< _lambda / _timeStep
   Ocean                         _ocean;              //< Initial conditions
};
#endif
