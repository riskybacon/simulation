//
//  ParticleSystemModel.h
//  Singularity
//
//  Created by Jeffrey Bowles on 1/16/13.
//  Copyright (c) 2013 Jeffrey Bowles. All rights reserved.
//

#ifndef __Singularity__ParticleSystemModel__
#define __Singularity__ParticleSystemModel__

#include <iostream>

#include <glm/glm.hpp>
#include <vector>
   
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

class ParticleSystemModel
{
public:
   /**
    * Constructor
    */
   ParticleSystemModel(unsigned int numParticles);
   
   /**
    * Destructor
    */
   ~ParticleSystemModel();
   
   /**
    * Initialize particles
    */
   void initParticles();
   
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
    * Get the model data
    */
   const glm::vec4* getData() const
   {
      return &_positions[0];
   }

   /**
    * Get the number of vertices
    */
   unsigned int getNumVertices() const
   {
      return _numParticles;
   }
   
   /**
    * Accessors for mass
    */
   
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
   
   // Particle data
   std::vector<glm::vec4>        _positions;
   std::vector<glm::vec4>        _velocities;
   std::vector<glm::vec4>        _initialPositions;
   std::vector<glm::vec4>        _initialVelocities;
   
   unsigned int                  _numParticles;       //< Number of particles
   
   float                         _singularityMass;    //< The mass of the singularity
   float                         _particleMass;       //< the mass of the particles
};
#endif /* defined(__Singularity__ParticleSystemModel__) */
