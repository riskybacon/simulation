//--------------------------------------------------------------------------------
// scene.h
//
// Top level object for directing inputs to the scene, updating models and
// drawing objects
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------
#ifndef _scene_h
#define _scene_h

#include <iostream>

#include "shader.h"

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ca_view_glsl.h"
#include "ca_model_glsl.h"
#include "ca_model_normals.h"

/**
 * Top level scene object. This handles drawing and updating the scene.
 * The draw() and update() methods are the most interesting part of this object
 */
class Scene
{
public:
   /**
    * Constructor
    * @param resourcePath
    *    Path to resources
    * @param width
    *    Width in pixels for the scene
    * @param height
    *    Height in pixels for the scene
    */
   Scene(const std::string& resourcePath, int width, int height);

   /**
    * Destructor
    */
   ~Scene();
   
   /**
    * Update the models in the scene. This does not draw the scene
    * but updates positions, models, etc.
    */
   void update();
   
   /**
    * Draw the scene
    */
   void draw();
   
   /**
    * Handle a pinch gesture. In this case, it zooms in and out
    */
   void pinchGesture(float scale, float velocity);
   
   /**
    * Handle a pan gesture. This rotates the model
    */
   void panGesture(float x, float y);
   
   /**
    * Handle a tap gesture
    */
   void tapGesture(float x, float y);
   
   /**
    * Set the projection matrix
    */
   void setProjection();
   
   /**
    * Handle window resize events and change the projection matrix
    */
   void resize(unsigned int width, unsigned int height);

   /**
    * Load the shaders for this scene
    */
   void loadShaders();

   /**
    * Add objects to the scene
    */
   void addObjects();

   /**
    * Reset the scene to initial conditions,
    * use 4 gaussians equally spaced
    */
   void resetToInitialConditionsGaussian();

   /**
    * Reset the scene to initial conditions, use
    * Phillips spectrum
    */
   void resetToInitialConditionsPhillips();

   /**
    * Reset the scene to initial conditions, use
    * Phillips spectrum and Gaussian
    */
   void resetToInitialConditionsGaussianAndPhillips();

   /**
    * Reset the camera to the initial position
    */
   void resetCameraToInitialState();
   
private:
   std::string       _resourcePath;    //< Path to the resources
   int               _width;           //< Width of the scene in pixels
   int               _height;          //< Height of the scene in pixels
   
   glm::mat4         _modelRot;
   glm::mat4         _modelTrans;
   glm::mat4         _view;
   glm::mat4         _proj;
   glm::vec3         _eye;
   glm::vec3         _center;
   glm::vec3         _up;

   glm::vec3         _eyeInitial;
   glm::vec3         _centerInitial;
   glm::vec3         _upInitial;

   glm::quat         _objQuat;         //< Quaternion that represents the rotation of the object
   
   GL::Program*      _caViewProg;      //< Shader program used when displaying the CA
   GL::Program*      _caUpdateProg;    //< Shader program used to calculate the heights in the mesh at each time step
   GL::Program*      _normalsProg;     //< Shader program used to calculate normals at each position on the mesh
   
   CAModelGLSL*      _caModel;
   CAViewGLSL*       _caView;
   CAModelNormals*   _caModelNormals;
   
   float             _zoomMax;
   float             _zoomMin;
   
   float             _fov;
   float             _depthMin;
   float             _depthMax;
};

#endif
