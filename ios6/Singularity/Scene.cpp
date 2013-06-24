//
//  Scene.cpp
//  CopyToPBO
//
//  Created by Jeffrey Bowles on 12/20/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Scene.h"
#include "ParticleSystemView.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;
using std::unique_ptr;
using std::shared_ptr;

Scene::Scene(const std::string& resourcePath, int width, int height)
:  _resourcePath     (resourcePath)
,  _width            (width)
,  _height           (height)
,  _eye              (0, 0, 10)
,  _up               (0, 1, 0)
,  _center           (0,0,0)
,  _trans            (0,0,0)
,  _zoomMax          (1000)
,  _zoomMin          (0.1)
,  _fov              (45.0)
,  _depthMin         (0.1)
,  _depthMax         (4000)
,  _settingsTapped   (false)
{
   glEnable(GL_TEXTURE_2D);
   glEnable(GL_BLEND);

   setProjection();
   loadShaders(resourcePath);
   addObjects();
   
   std::cout << "Scene::Scene(width, height): " << width << "," << height << std::endl;
   _view = glm::lookAt(_eye, _center, _up);
   
}

Scene::~Scene()
{
}

void Scene::loadShaders(const std::string& resourcePath)
{
   // Set the location of the shader files
   std::string tqVertFile = resourcePath + "/textured_quad.vsh";
   std::string tqFragFile = resourcePath + "/textured_quad.fsh";
   std::string psVertFile = resourcePath + "/render_vert.vsh";
   std::string psFragFile = resourcePath + "/render_frag.fsh";

   try
   {
      _tqProg = unique_ptr<GL::Program>(new GL::Program(tqVertFile, tqFragFile));
      _psProg = unique_ptr<GL::Program>(new GL::Program(psVertFile, psFragFile));
   }
   catch(const std::runtime_error& err)
   {
      std::cerr << err.what() << std::endl;
   }
}

void Scene::addObjects()
{
   try
   {
      _particleSystemModel = shared_ptr<ParticleSystemModel>(new ParticleSystemModel(5000));
      _particleSystemView  = unique_ptr<ParticleSystemView> (new ParticleSystemView(_psProg->getAttribLocation("pos"), _particleSystemModel));
      _settingsIcon        = unique_ptr<SettingsIcon>       (new SettingsIcon(_tqProg->getAttribLocation("pos"),
                                                                              _tqProg->getAttribLocation("tc")));
   }
   catch(const std::runtime_error& err)
   {
      std::cerr << err.what() << std::endl;
   }
}
void Scene::pinchGesture(float scale, float velocity)
{
   _modelTrans[3][3] -= scale * 0.1 * velocity;

   // Clamp the model Z coord between [_zoomMin, _zoomMax]
   _modelTrans[3][3] = _modelTrans[3][3] < _zoomMin ? _zoomMin : _modelTrans[3][3];
   _modelTrans[3][3] = _modelTrans[3][3] > _zoomMax ? _zoomMax : _modelTrans[3][3];
}

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

void Scene::tapGesture(float x, float y)
{
#if 0
   vec4 pos(x, y, 0, 1);
   
   mat4 bias = glm::translate(mat4(), vec3(-1, 1, 0));
#endif
   
#if 0
   // Check to see if the settings icon was clicked
   mat4 scaleAndBias = glm::scale(bias, vec3(2.0 / _width, -2.0 / _height, 1.0));
   
   // Check to see if the settings icon was clicked
   pos = _settingsIconModelInv * scaleAndBias * pos;
   
   if(pos.x >= -1.0 && pos.x <= 1.0 && pos.y >= -1.0 && pos.y <= 1.0)
   {
      _settingsTapped = true;
      std::cerr << "tapped settings" << std::endl;
   }
#endif
}

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

   _settingsIconModel = qTrans * qScale;
   
   qScale = glm::scale(mat4(), vec3(1.0 / qWidth, 1.0 / qHeight, 1.0));
   qTrans = glm::translate(mat4(), vec3(-qTransX, -qTransY, 1.0));
   
   _settingsIconModelInv = qScale * qTrans;
}

void Scene::resize(unsigned int width, unsigned int height)
{
   _width = width;
   _height = height;
   setProjection();
}

void Scene::update()
{
   _particleSystemModel->update();
   _particleSystemView->update();
}

void Scene::draw()
{
   glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   glm::mat4 mv;

   // Draw the particle system
   mv = _view * _modelTrans * _modelRot;
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);
   _psProg->bind();
   _psProg->setUniform("mv", mv);
   _psProg->setUniform("proj", _proj);
   _particleSystemView->draw();
   glDisable(GL_BLEND);
   
#if 0
   // Draw the settings icon
   _tqProg->bind();
   _tqProg->setUniform("mv", _settingsIconModel);
   _tqProg->setUniform("proj", mat4());
   _settingsIcon->draw();
   _tqProg->release();
#endif
}