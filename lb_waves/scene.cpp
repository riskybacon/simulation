//--------------------------------------------------------------------------------
// scene.cpp
//
// Top level object for directing inputs to the scene, updating models and
// drawing objects
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------
#include "scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>
#include "scene.h"
#include <unistd.h>

// Bring often used glm symbols into namespace
using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

/*
 * Constructor
 * @param resourcePath
 *    Path to resources
 * @param width
 *    Width in pixels for the scene
 * @param height
 *    Height in pixels for the scene
 */
Scene::Scene(const std::string& resourcePath, int width, int height)
:  _resourcePath     (resourcePath)
,  _width            (width)
,  _height           (height)
,  _eye              (0, 7, 75)
,  _center           (0,0,0)
,  _up               (0, 1, 0)
,  _eyeInitial       (_eye)
,  _centerInitial    (_center)
,  _upInitial        (_up)
,  _caViewProg       (NULL)
,  _caUpdateProg     (NULL)
,  _normalsProg      (NULL)
,  _caModel          (NULL)
,  _caView           (NULL)
,  _caModelNormals   (NULL)
,  _zoomMax          (1000)
,  _zoomMin          (0.1)
,  _fov              (45.0)
,  _depthMin         (0.1)
,  _depthMax         (4000)
{
   setProjection();
   loadShaders();
   addObjects();
   
   std::cout << "Scene::Scene(width, height): " << _width << "," << _height << std::endl;
   _view = glm::lookAt(_eye, _center, _up);
}

/*
 * Destructor
 */
Scene::~Scene()
{
   delete _caModel;
   delete _caView;
   delete _caModelNormals;
   delete _caViewProg;
   delete _caUpdateProg;
   delete _normalsProg;
}

/*
 * Load the shaders for this scene
 */
void Scene::loadShaders()
{
   // Set the location of the shader files
   std::string waveVertFile     = _resourcePath + "/wave_vert.c";
   std::string waveFragFile     = _resourcePath + "/wave_frag.c";

   std::string normalsVertFile  = _resourcePath + "/compute_vert.c";
   std::string normalsFragFile  = _resourcePath + "/compute_normals_frag.c";
   
   std::string caUpdateVertFile = _resourcePath + "/compute_vert.c";
   std::string caUpdateFragFile = _resourcePath + "/ca_update_frag.c";

   try
   {
      GL::Program* newProg;
      
      // Load wave render GLSL program
      newProg = new GL::Program(waveVertFile, waveFragFile);
      if(_caViewProg != NULL)
      {
         delete _caViewProg;
      }
      _caViewProg = newProg;
      
      // Load the shader program that computes the mesh's normals
      newProg = new GL::Program(normalsVertFile, normalsFragFile);
      if(_normalsProg != NULL)
      {
         delete _normalsProg;
      }
      _normalsProg = newProg;

      // Tell the computation object about the program
      if(_caModelNormals != NULL)
      {
         _caModelNormals->setProgram(_normalsProg);
      }
      
      // Load the CA update program
      newProg = new GL::Program(caUpdateVertFile, caUpdateFragFile);
      if(_caUpdateProg != NULL)
      {
         delete _caUpdateProg;
      }
      _caUpdateProg = newProg;
      
      // Tell the CA model about the new compute program
      if(_caModel != NULL)
      {
         _caModel->setProgram(_caUpdateProg);
      }
   }
   catch(const std::runtime_error& err)
   {
      std::cerr << err.what() << std::endl;
      if(_caViewProg == NULL || _normalsProg == NULL)
      {
         exit(0);
      }
   }
}

/*
 * Add objects to the scene
 */
void Scene::addObjects()
{
   try
   {
      //      _caModel        = new CAModelGLSL(ivec2(256, 256), vec2(-20, -20), vec2(20,20), 2048, 1.0f / 10.0f, _caUpdateProg);
      // Works:
      //o+_caModel        = new CAModelGLSL(ivec2(64, 64), vec2(-20, -20), vec2(20,20), 64, 1.0f / 64.0f, _caUpdateProg);
      _caModel        = new CAModelGLSL(ivec2(128, 128), vec2(-20, -20), vec2(20,20), 64, 1.0f / 128.0f, _caUpdateProg);
      //      _caModel        = new CAModelGLSL(ivec2(256, 256), vec2(-20, -20), vec2(20,20), 128, 1.0f / 128.0f, _caUpdateProg);

      _caModelNormals = new CAModelNormals(_caModel, _normalsProg);
      _caView         = new CAViewGLSL(_caViewProg->getAttribLocation("posIdx"),
                                       GL_TEXTURE0,
                                       GL_TEXTURE1,
                                       _caModel,
                                       _caModelNormals);
   }
   catch(const std::runtime_error& err)
   {
      std::cerr << err.what() << std::endl;
   }
}

/*
 * Handle a pinch gesture. In this case, it zooms in and out
 */
void Scene::pinchGesture(float scale, float velocity)
{
   _modelTrans[3][3] -= scale * 0.1 * velocity;

   // Clamp the model Z coord between [_zoomMin, _zoomMax]
   _modelTrans[3][3] = _modelTrans[3][3] < _zoomMin ? _zoomMin : _modelTrans[3][3];
   _modelTrans[3][3] = _modelTrans[3][3] > _zoomMax ? _zoomMax : _modelTrans[3][3];
}

/*
 * Create a quaternion with the specified angle about an axis
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

/**
 * Handle a pan gesture. This rotates the model
 */
void Scene::panGesture(float x, float y)
{
   const float sensitivity = 0.005; // * M_PI / 180.0f;
   
   const glm::vec3 yAxis(0, 1, 0);
   const glm::vec3 xAxis(1, 0, 0);
   
   // This operation looks backwards, but movement in the x direction on
   // the touch screen rotates the model about the y-axis
   glm::quat yRot = createQuat(x * sensitivity, yAxis);
   glm::quat xRot = createQuat(y * sensitivity, xAxis);
   
   _objQuat = glm::normalize(yRot * xRot * _objQuat);
   _modelRot = glm::mat4_cast(_objQuat);
}

/*
 * Handle a tap gesture
 */
void Scene::tapGesture(float x, float y)
{
}

/*
 * Set the projection matrix
 */
void Scene::setProjection()
{
   _proj = glm::perspective(_fov, float(_width) / float(_height), _depthMin, _depthMax);
   glViewport(0, 0, _width, _height);
   
   // Set the size for the settings quad
   float size = 0.03;
   float qWidth = size;
   float aspectRatio = float(_width) / float(_height);
   float qHeight = size * aspectRatio;
   
   float qTransX = 1.0 - qWidth - size * 0.5;
   float qTransY = -1.0 + qHeight + size * 0.5;
   
   mat4 qScale = glm::scale(mat4(), vec3(qWidth, qHeight, 1.0));
   mat4 qTrans = glm::translate(mat4(), vec3(qTransX, qTransY, 0));

   qScale = glm::scale(mat4(), vec3(1.0 / qWidth, 1.0 / qHeight, 1.0));
   qTrans = glm::translate(mat4(), vec3(-qTransX, -qTransY, 1.0));
}

/*
 * Handle window resize events and change the projection matrix
 */
void Scene::resize(unsigned int width, unsigned int height)
{
   _width = width;
   _height = height;
   setProjection();
}

/*
 * Reset the scene to the initial conditions
 */
void Scene::resetToInitialConditionsGaussian()
{
   _caModel->initialStateGaussian();
   _caModel->uploadInitialConditions();
   _caModelNormals->update();
}

/*
 * Reset the scene to the initial conditions
 */
void Scene::resetToInitialConditionsPhillips()
{
   _caModel->initialStatePhillips();
   _caModel->uploadInitialConditions();
   _caModelNormals->update();
}

/*
 * Reset the scene to the initial conditions
 */
void Scene::resetToInitialConditionsGaussianAndPhillips()
{
   _caModel->initialStateGaussianAndPhillips();
   _caModel->uploadInitialConditions();
   _caModelNormals->update();
}

/*
 * Reset the camera to the initial position
 */
void Scene::resetCameraToInitialState()
{
   _eye    = _eyeInitial;
   _up     = _upInitial;
   _center = _centerInitial;
   _view   = glm::lookAt(_eye, _center, _up);
   _modelRot = mat4();
   _modelTrans = mat4();
}

/*
 * Update the models in the scene. This does not draw the scene
 * but updates positions, models, etc.
 */
void Scene::update()
{
   _caModelNormals->update();
   _caModel->update();
}


/*
 * Draw the OpenGL scene
 */
void Scene::draw()
{
   // Sky blue background
   glClearColor(0.53f, 0.81f, 0.92f, 0.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);
   glViewport(0, 0, _width, _height);
   
   glm::mat4 model = _modelTrans * _modelRot;
   
   // Calculate the inverse transpose for use with normals

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, _caModel->getCurPositionID());
   
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, _caModelNormals->getTexID());
   _caViewProg->bind();
   _caViewProg->setUniform("proj",    _proj);
   _caViewProg->setUniform("model",   model);
   _caViewProg->setUniform("view",    _view);
   _caViewProg->setUniform("pos",     0);
   _caViewProg->setUniform("normals", 1);
   _caView->draw();
}
