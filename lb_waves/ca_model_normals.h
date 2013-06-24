//--------------------------------------------------------------------------------
// ca_model_normals.cpp
//
// Calculates the normal vectors for each lattice point. This is needed for
// proper shading.
//
// TODO: a more efficient method of calculating normals.
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------
#ifndef _ca_model_normals_h
#define _ca_model_normals_h

#include <glm/glm.hpp>
#include "ca_model_glsl.h"
#include "opengl.h"
#include "shader.h"
#include <string>

/**
 * Find the normals in a mesh. This is calculated using
 * a fragment shader
 */
class CAModelNormals
{
public:
   /**
    * Constructor. 
    */
   CAModelNormals(const CAModelGLSL* model, GL::Program* computeProg);
   
   /**
    * Update the model to the next time step
    */
   void update();
   
   /**
    * Get the current position texture ID
    */
   const GLuint getTexID() const
   {
      return _normTexID;
   }

   /**
    * Set the compute shader
    */
   void setProgram(GL::Program* prog)
   {
      _computeProg = prog;
   }
   
protected:

   /**
    * Create an FBO that has an RGBA 32-bit floating point texture
    * and a texture for holding depth values
    */
   void createFBO();
   
   /**
    * Set the initial state of the CA
    */
   void initialState();
   
   /**
    * Initialize the computation quad. The quad covers the entire "screen"
    */
   void initComputeQuad();
   
   /**
    * Load the shaders
    */
   void loadShaders();

   /**
    * Check the status of an FBO
    */
   void fboStatus();

private:
   const CAModelGLSL*            _model;              //< Pointer to the model
   GL::Program*                  _computeProg;        //< Computation shader
   GLuint                        _normTexID;          //< Texture IDs for the normal textures
   GLuint                        _fboID;              //< Frame buffer object handles
   GLuint                        _computeVAO;         //< Vertex array object for the compute quad
   GLuint                        _computeBuf;         //< Buffer object for compute quad positions
   GLuint                        _posAttr;            //< Location of position attribute
   GLuint                        _tcAttr;             //< Location of texture coordinate attribute
   float                         _deltaS;             //< Distance in S to the next texel
   float                         _deltaT;             //< Distance in T to the next texel
};
#endif
