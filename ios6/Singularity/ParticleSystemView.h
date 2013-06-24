//
//  ParticleSystemView.h
//  Singularity
//
//  Created by Jeffrey Bowles on 1/16/13.
//  Copyright (c) 2013 Jeffrey Bowles. All rights reserved.
//

#ifndef __Singularity__ParticleSystemView__
#define __Singularity__ParticleSystemView__

#include <iostream>
#include "OpenGL.h"
#include <memory>

class ParticleSystemModel;

class ParticleSystemView
{
public:
   
   /**
    * Constructor
    */
   ParticleSystemView(GLuint posAttr, std::shared_ptr<ParticleSystemModel>);
   
   /**
    * Destructor
    */
   ~ParticleSystemView();
   
   /**
    * Draw the particle system
    */
   void draw();
   
   /**
    * Update the buffers with the data
    */
   void update();
   
private:
   GLuint                                 _posAttr;            //< Location of the position attribute
   std::shared_ptr<ParticleSystemModel>   _model;
   GLuint                                 _pVAO;               //< Vertex array object for the positions
   GLuint                                 _pBO;                //< Buffer object for the positions
};

#endif /* defined(__Singularity__ParticleSystemView__) */
