//--------------------------------------------------------------------------------
// ca_view_glsl.h
//
// The OpenGL view of the cellular automata.
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------
#ifndef _ca_view_glsl_h
#define _ca_view_glsl_h

class CAModelGLSL;
class CAModelNormals;

#include "opengl.h"
#include <glm/glm.hpp>
#include <vector>

/**
 * The OpenGL representation of the cellular automata model
 * This particular view uses vertex texture fetch to get the positions
 * of the vertices. The texture map with the positions is calculated
 * in CAModelGLSL
 */
class CAViewGLSL
{
public:
   /**
    * Constructor
    */
   CAViewGLSL(GLuint posAttr, GLuint posTexUnit, GLuint normTexUnit, CAModelGLSL* model, CAModelNormals* normals);
   
   /**
    * Destructor
    */
   ~CAViewGLSL();
   
   /**
    * Draw the CA
    */
   void draw();
   
   /**
    * Update the view of the CA. This updates the normals for every point.
    */
   void update();
   
private:
   GLuint                     _posAttr;            //< Location of the position attribute
   GLuint                     _posTexUnit;         //< Texture unit for the positions
   GLuint                     _normTexUnit;        //< Texture unit for the normals
   CAModelGLSL*               _model;              //< Data model
   CAModelNormals*            _normals;            //< Normals for the mesh
   GLuint                     _pVao;               //< Vertex array object for the positions
   GLuint                     _posBuf;             //< Buffer object for the positions
   GLuint                     _idxBuf;             //< Buffer object for the vertex indices
   std::vector<GLuint>        _indices;            //< Set of indices - the order to draw the positions
   std::vector<glm::vec2>     _positions;          //< Set of texture map positions

   
};
#endif
