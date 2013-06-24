//--------------------------------------------------------------------------------
// ca_model_normals.h
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
#include <iostream>
#include <stdexcept>

#include "ca_model_normals.h"

using glm::ivec2;
using glm::vec2;
using glm::vec4;
using std::vector;

/**
 * Constructor.
 */
CAModelNormals::CAModelNormals(const CAModelGLSL* model, GL::Program* computeProg)
: _model       (model)
, _computeProg (computeProg)
{
   
   _posAttr = _computeProg->getAttribLocation("pos");
   _tcAttr = _computeProg->getAttribLocation("texcoord");

   // Create computation data structures on the GPU
   createFBO();
   // Set up the computation quad
   initComputeQuad();
   
}

/*
 * Check the status of an FBO
 */
void CAModelNormals::fboStatus(void)
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
 * Initialize the computation quad. The quad covers the entire "screen"
 */
void CAModelNormals::initComputeQuad()
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
   
   glBindBuffer(GL_ARRAY_BUFFER, _computeBuf);
   // Ask the GL to allocate a buffer that can hold the positions and texcoords
   glBufferData(GL_ARRAY_BUFFER, bufsize, NULL, GL_STATIC_DRAW);
   // Copy in the position data starting at the beginning of the buffer
   glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, &positions[0]);
   // Copy in the texcoord data starting at the posSize offset in the buffer
   glBufferSubData(GL_ARRAY_BUFFER, posSize, tcSize, &texcoords[0]);
   
   // The pos attribute gets its data from the start of the buffer
   glVertexAttribPointer(_posAttr, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
   glEnableVertexAttribArray(_posAttr);
   
   // The texcoord attribute gets its data starting at the posSize offset in the buffer
   glVertexAttribPointer(_tcAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)posSize);
   glEnableVertexAttribArray(_tcAttr);
   
   glBindVertexArray(0);
}

/*
 * Create an FBO that has an RGBA 32-bit floating point texture
 * and a texture for holding depth values
 */
void CAModelNormals::createFBO()
{
   GL_ERR_CHECK();
   try
   {
      const ivec2 size = _model->getLatticeSize();

      _deltaT = 1.0f / size.y;
      _deltaS = 1.0f / size.x;
      
      glGenTextures(1, &_normTexID);
      glGenFramebuffers(1, &_fboID);
      GL_ERR_CHECK();
      
      // Check for errors during texture and framebuffer generation
      bool fail = false;
     if(_normTexID <= 0)
     {
        std::cerr << "Texture not generated for _posTexID" << std::endl;
        fail = true;
     }
      
      
      if(_fboID <= 0)
      {
         std::cerr << "Framebuffer not generated for _fboID" << std::endl;
         fail = true;
      }

      vector<vec4> normals(size.x * size.y, vec4(1,0,0,0));
      
      // Create the compute textures and attach them to the framebuffer object
      glBindTexture(GL_TEXTURE_2D, _normTexID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_FLOAT, &normals[0]);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      
         // Set up the framebuffer object (FBO)
      glBindFramebuffer(GL_FRAMEBUFFER, _fboID);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _normTexID, 0);
      
      vector<GLuint> drawBuffers =
      {
         GL_COLOR_ATTACHMENT0,
      };
      
      // Set the drawing buffer for this FBO
      glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);

      // Set the reading buffer for this FBO
      glReadBuffer(GL_NONE);
      
      // Check the status
      fboStatus();
      
      // Unbind
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
 * Update the model
 */
void CAModelNormals::update()
{
   glDisable(GL_BLEND);
   glDisable(GL_DEPTH_TEST);
   const ivec2 size = _model->getLatticeSize();
   
   _computeProg->bind();
   
   // Set the viewport to be the size of the destination texture
   glViewport(0, 0, size.x, size.y);
   
   // Bind the framebuffer object
   glBindFramebuffer(GL_FRAMEBUFFER, _fboID);
   
   // Bind the source particle attribute textures
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, _model->getCurPositionID());
   _computeProg->setUniform("inPos", 0);
   _computeProg->setUniform("deltaS", _deltaS);
   _computeProg->setUniform("deltaT", _deltaT);
   
   // Draw the quad that covers the entire FBO
   glBindVertexArray(_computeVAO);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   
   // Reset the VAO, shader and framebuffer object state back to
   // the default
   glBindVertexArray(0);
   _computeProg->release();
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
