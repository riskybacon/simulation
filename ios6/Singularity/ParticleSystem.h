#if 0
//
//  ParticleSystem.h
//  CopyToPBO
//
//  Created by Jeffrey Bowles on 12/20/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

#ifndef __CopyToPBO__ParticleSystem__
#define __CopyToPBO__ParticleSystem__

#include <iostream>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <vector>

#include "Shader.h"


/**
 * Structures for passing around position and velocity state
 * and derivatives for use with RK4
 */
struct Derivative
{
   glm::vec3 dx;
   glm::vec3 dv;
};

struct State
{
   glm::vec3 x;
   glm::vec3 v;
};

class ParticleSystem
{
public:
#if 0
   /**
    * Constructor
    */
   ParticleSystem(const std::string& resourcePath, unsigned int numParticles, unsigned int width, unsigned int height);
#endif
   
   /**
    * Constructor
    */
   ParticleSystem(GLuint posAttr, unsigned int numParticles);
   
   /**
    * Destructor
    */
   ~ParticleSystem();
   
   /**
    * Initialize particles
    */
   void initParticles();
   
#if 0
   /**
    * Load the shaders
    */
   void reloadShaders();
#endif
   
   /**
    * Initialize a single particle
    *
    * @param i
    *    The index of the particle to initialize
    */
   void initParticle(int i);

   /**
    * Initialize particles
    */
   void initParticles(unsigned int numParticles);

   /**
    * Initialize the computation quad. The quad covers the entire "screen"
    */
   void initComputeQuad();

   /**
    * Update particle positions
    */
   void update();

   /**
    * Draw this object
    */
   void draw(const glm::mat4& mv, const glm::mat4& proj);
   
   /**
    * Draw the particle system
    */
   void draw();

   float getSingularityMass() const
   {
      return _singularityMass;
   }
   
   void setSingularityMass(float mass)
   {
      _singularityMass = mass;
   }
   
   float getParticleMass() const
   {
      return _particleMass;
   }
   
   void setParticleMass(float mass)
   {
      std::cerr << "setParticleMass(" << mass << ")" << std::endl;
      _particleMass = mass;
   }
   
private:
   GLuint                        _pVAO;               //< Vertex array object for the positions
   GLuint                        _pBO;                //< Buffer object for the positions
   GLuint                        _posAttr;            //< Location of the position attribute
   
   // Particle data
   std::vector<glm::vec4>        _positions;
   std::vector<glm::vec4>        _velocities;
   std::vector<glm::vec4>        _initialPositions;
   std::vector<glm::vec4>        _initialVelocities;
   
   unsigned int                  _numParticles;       //< Number of particles
   
   float                         _singularityMass;    //< The mass of the singularity
   float                         _particleMass;       //< the mass of the particles
   
   
};
#endif /* defined(__CopyToPBO__ParticleSystem__) */
#endif
