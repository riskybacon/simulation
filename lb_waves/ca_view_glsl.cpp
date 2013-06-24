//--------------------------------------------------------------------------------
// ca_view_glsl.cpp
//
// The OpenGL view of the cellular automata.
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------
#include "ca_view_glsl.h"
#include "ca_model_glsl.h"
#include "ca_model_normals.h"
#include <iostream>

using glm::ivec2;
using glm::vec2;
using std::vector;

/*
 * Constructor
 *
 * Initialize vertex array objects, vertex buffer objects,
 */
CAViewGLSL::CAViewGLSL(GLuint posAttr, GLuint posTexUnit, GLuint normTexUnit, CAModelGLSL* model, CAModelNormals* normals)
: _posAttr     (posAttr)
, _posTexUnit  (posTexUnit)
, _normTexUnit (normTexUnit)
, _model       (model)
, _normals     (normals)  
, _pVao        (0)
, _posBuf      (0)
, _idxBuf      (0)
{
   const ivec2 size = _model->getLatticeSize();
   
   // Set up a Vertex Array Object that contains the texture coordinates needed
   // to read the positions out of the texture map
   _positions.resize(size.x * size.y);
   
   // The width and height of a texel in the texture map
   float texelWidth  = 1.0f / size.x;
   float texelHeight = 1.0f / size.y;
   
   // The amount to bias the texture coordinate so that
   // the coordinate points to the center of the texel and
   // does not point to the border. This ensures that the
   // texture unit fetches the value of the desired texel
   // and not it's neighbor's value
   float s_bias = texelWidth * 0.5f;
   float t_bias = texelHeight * 0.5f;
   
   // Create list of indices into the texture map
   for(int i = 0; i < size.x; ++i)
   {
      for(int j = 0; j < size.y; ++j)
      {
         // Get the s and t coord in the (0,1) range
         float s = float(i) / size.x;
         float t = float(j) / size.y;
         
         // Bias the coordinate so that it points to the center of the texel
         s += s_bias;
         t += t_bias;
         
         // Put the coordinate into the array
         size_t idx = i * size.x + j;
         _positions.at(idx) = vec2(s,t);
      }
   }

   // Each set of 4 points in the mesh make up two triangles
   for(int t = 0; t < size.y - 1; t++)
   {
      for(int s = 0; s < size.x - 1; s++)
      {
         _indices.push_back(t       * size.y + s);
         _indices.push_back((t + 1) * size.y + s);
         _indices.push_back(t       * size.y + (s + 1));

         _indices.push_back(t       * size.y + (s + 1));
         _indices.push_back((t + 1) * size.y + s);
         _indices.push_back((t + 1) * size.y + (s + 1));
      }
   }
   
   // Create the vertex array object that will be used to draw the triangles
   glGenVertexArrays(1, &_pVao);
   glBindVertexArray(_pVao);
   
   glGenBuffers(1, &_idxBuf);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _idxBuf);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), &_indices[0], GL_STATIC_DRAW);
   
   glGenBuffers(1, &_posBuf);
   glBindBuffer(GL_ARRAY_BUFFER, _posBuf);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * _positions.size(), &_positions[0], GL_STATIC_DRAW);
   glVertexAttribPointer(_posAttr, 2, GL_FLOAT, GL_FALSE, 0, NULL);
   glEnableVertexAttribArray(_posAttr);
   glBindVertexArray(0);
}

/*
 * Destructor
 */
CAViewGLSL::~CAViewGLSL()
{
   glDeleteVertexArrays(1, &_pVao);
   glDeleteBuffers(1, &_idxBuf);
   glDeleteBuffers(1, &_posBuf);
}

/*
 * Draw the CA
 */
void CAViewGLSL::draw()
{
   // Multi-texturing - there are two textures bound. These textures
   // contain the positions and the normals
   glActiveTexture(_posTexUnit);
   glBindTexture(GL_TEXTURE_2D, _model->getCurPositionID());

   glActiveTexture(_normTexUnit);
   glBindTexture(GL_TEXTURE_2D, _normals->getTexID());

   // Bind the vertex array that indexes into the textures
   glBindVertexArray(_pVao);

   // Draw the CA as a set of triangles
   glDrawElements(GL_TRIANGLES,          // Primitive to draw
                  _indices.size(),       // Number of indices
                  GL_UNSIGNED_INT,       // Data type of index values
                  NULL);                 // Pointer to the data (NULL if data on GPU already)
   GL_ERR_CHECK();
  
}

/*
 * Update the view of the CA. This updates the normals for every point
 */
void CAViewGLSL::update()
{
   // Set the viewport to the size
}


