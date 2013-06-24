//
//  Scene.h
//  CopyToPBO
//
//  Created by Jeffrey Bowles on 12/20/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

#ifndef __CopyToPBO__Scene__
#define __CopyToPBO__Scene__

#include <iostream>

#include "Shader.h"

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "SettingsIcon.h"
#include "ParticleSystemModel.h"

class ParticleSystemView;

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
    * Update the scene.
    */
   void update();
   
   /**
    * Draw the scene
    */
   void draw();
   
   /**
    * Handle a pinch gesture
    */
   void pinchGesture(float scale, float velocity);
   
   /**
    * Handle a pan gesture
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
    * Handle rotation / scene resize events
    */
   void resize(unsigned int width, unsigned int height);

   /**
    * Load the shaders for this scene
    *
    * @param   resourcePath
    *    The path to the shaders
    */
   void loadShaders(const std::string& resourcePath);

   /**
    * Add objects to the scene
    */
   void addObjects();

   /**
    * Was the settings icon tapped?
    */
   bool settingsTapped() {
      if(_settingsTapped) {
         _settingsTapped = false;
         return true;
      }
      return false;
         
   }
   
   float getSingularityMass() const
   {
      return _particleSystemModel->getSingularityMass();
   }
   
   void setSingularityMass(float mass)
   {
      _particleSystemModel->setSingularityMass(mass);
   }
   
   float getParticleMass() const
   {
      return _particleSystemModel->getParticleMass();
   }
   
   void setParticleMass(float mass)
   {
      _particleSystemModel->setParticleMass(mass);
   }
   
private:
   std::string    _resourcePath;    //< Path to the resources
   int            _width;           //< Width of the scene in pixels
   int            _height;          //< Height of the scene in pixels
   
   glm::mat4      _modelRot;
   glm::mat4      _modelTrans;
   glm::mat4      _view;
   glm::mat4      _proj;
   glm::mat4      _settingsIconModel;     //< Model matrix for the settings icon
   glm::mat4      _settingsIconModelInv;  //< Inverse model matrix for the settings icon
   glm::vec3      _eye;
   glm::vec3      _center;
   glm::vec3      _up;
   
   glm::vec3      _trans;           //< Translation for the model
   glm::quat      _objQuat;         //< Quaternion that represents the rotation of the object
   
   std::unique_ptr<GL::Program>           _tqProg;
   std::unique_ptr<GL::Program>           _psProg;
   std::shared_ptr<ParticleSystemModel>   _particleSystemModel;
   std::unique_ptr<ParticleSystemView>    _particleSystemView;
   std::unique_ptr<SettingsIcon>          _settingsIcon;
   
   float          _zoomMax;
   float          _zoomMin;
   
   float          _fov;
   float          _depthMin;
   float          _depthMax;
   bool           _settingsTapped;  //< True if the settings icon was tapped
};

#endif /* defined(__CopyToPBO__Scene__) */
