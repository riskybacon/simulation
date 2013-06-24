#if 0
//
//  ParticleSystem.cpp
//  CopyToPBO
//
//  Created by Jeffrey Bowles on 12/20/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

#include "ParticleSystem.h"

using std::unique_ptr;
using std::vector;
using glm::vec4;
using glm::vec3;
using glm::vec2;

#if 0
const float m1 = 9.5e9;
const float m2 = 1e5;
const float invM1 = 1.0 / m1;
const float invM2 = 1.0 / m2;
#endif

//----------------------------------------------------------------------
// Calculate the magnitude of the gravitational force between
// two masses.
//
// Input:  m1 - mass in Kg
//         m2 - mass in Kg
//         r  - distance in meters
//
// Return: magnitude of attractive force in Newtons
// floating point ops: 2
//----------------------------------------------------------------------
float gravity(float m1, float m2, float r)
{
   float G = 6.67e-11f; // Nm^2/kg^2
   return G * m1 * m2 / (r * r);
}

//----------------------------------------------------------------------
// Calculate the acceleration vector
//
// Input:  p0 - position in space (m)
//         v0 - velocity (m / s^2)
//         t  - time (s)
//
// m1 - singularity mass
// m2 - particle mass
// Output: acceleration vector (m/s)
// floating point ops: 6
//----------------------------------------------------------------------
vec3 acceleration(State state, float t, float m1, float m2)
{
   float g = gravity(m1, m2, glm::length(state.x));
   vec3 force = -g * glm::normalize(state.x);
   
   // Acceleration vector. F = ma => a = F/m
   return force / m2;
}

//----------------------------------------------------------------------
// RK4 functions
// floating point ops: 6
//----------------------------------------------------------------------
Derivative evaluate(State initial, float t, float m1, float m2)
{
   Derivative OUT;
   OUT.dx = initial.v;
   OUT.dv = acceleration(initial, t, m1, m2);
   return OUT;
}

// floating point ops: 11
Derivative evaluate(State initial, float t, float dt, Derivative d, float m1, float m2)
{
   State state;
   state.x = initial.x + d.dx * dt;
   state.v = initial.v + d.dv * dt;
   
   Derivative OUT;
   OUT.dx = state.v;
   OUT.dv = acceleration(state, t+dt, m1, m2);
   
   return OUT;
}

// 6 + 11 + 11 + 11 + 10 + 4 = 53
void integrate(State& state, float t, float dt, float m1, float m2)
{
   Derivative a = evaluate(state, t, m1, m2);
   Derivative b = evaluate(state, t, dt*0.5f, a, m1, m2);
   Derivative c = evaluate(state, t, dt*0.5f, b, m1, m2);
   Derivative d = evaluate(state, t, dt, c, m1, m2);
   
   vec3 dxdt = 0.166666667f * (a.dx + 2.0f*(b.dx + c.dx) + d.dx);
   vec3 dvdt = 0.166666667f * (a.dv + 2.0f*(b.dv + c.dv) + d.dv);
   
   state.x = state.x + dxdt*dt;
   state.v = state.v + dvdt*dt;
}

/**
 * Constructor
 *
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
ParticleSystem::ParticleSystem(GLuint posAttr, unsigned int numParticles)
:  _posAttr       (posAttr)
,  _numParticles  (numParticles)
,  _singularityMass  (9.5e9)
,  _particleMass     (1e5)
{
   // Initial particle system, textures and FBOs
   initParticles(numParticles);
}

/*
 * Destructor
 */
ParticleSystem::~ParticleSystem()
{
   glDeleteBuffers(1, &_pBO);
   glDeleteVertexArraysOES(1, &_pVAO);
}

/*
 * Initialize a single particle
 *
 * @param i
 *    The index of the particle to initialize
 */
void ParticleSystem::initParticle(int i)
{
   // Determine the initial velocity. The magnitude of all velocities is the
   // same and determined by the variable r, but the directions are random
   
   // Define the velocity in terms of polar coordinates
   float theta = 2 * M_PI * rand() / RAND_MAX;
   float phi = 2 * M_PI * rand() * RAND_MAX;
   float r = 3.51f;
   
   // Translate from polar to cartesian
   float sin_theta = sin(theta);
   float cos_theta = cos(theta);
   float sin_phi   = sin(phi);
   float cos_phi   = cos(phi);
   float x = r * cos_phi * sin_theta;
   float y = r * sin_phi * sin_theta;
   float z = r * cos_theta;
   
   _velocities.at(i) = vec4(x,y,z,0);
   _initialVelocities.at(i) = _velocities.at(i);
   float height = 0.1;
   
   _positions.at(i)  = vec4(height, 0, 0, 1.0f);
   _initialPositions.at(i) = _positions.at(i);
}


/*
 * Initialize particles
 */
void ParticleSystem::initParticles(unsigned int numParticles)
{
   _positions.resize(numParticles);
   _velocities.resize(numParticles);
   _initialPositions.resize(numParticles);
   _initialVelocities.resize(numParticles);
   
   for(size_t i = 0; i < _numParticles; ++i)
   {
      initParticle(i);
   }
   
   glGenVertexArrays(1, &_pVAO);
   glBindVertexArray(_pVAO);
   glGenBuffers(1, &_pBO);
   glBindBuffer(GL_ARRAY_BUFFER, _pBO);
   // GL_STATIC_DRAW has best performance. One would expect GL_STATIC_COPY would have the best performance,
   // but it is not the case.
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * _numParticles, &_positions[0], GL_DYNAMIC_DRAW);
   glVertexAttribPointer(_posAttr, 4, GL_FLOAT, GL_FALSE, 0, NULL);
   glEnableVertexAttribArray(_posAttr);
   glBindVertexArray(0);
}


/**
 * Update particle positions
 */
void ParticleSystem::update()
{
   // Time step
   static float t = 0.01;
   
   // Update each particle
   for(int i = 0; i < _numParticles; i++)
   {
      // Get the new position using RK4 (4th order Runge-Kutta method)
      State state;
      state.x = vec3(_positions[i].x, _positions[i].y, _positions[i].z);
      state.v = vec3(_velocities[i].x, _velocities[i].y, _velocities[i].z);
      integrate(state, 0, t, _singularityMass, _particleMass);
      _positions[i] = vec4(state.x, 1);
      _velocities[i] = vec4(state.v, 0);
      
      // If a particle gets too far away, reset the position and velocity
      if(glm::length(vec3(_positions[i].x, _positions[i].y, _positions[i].z)) > 100.0f)
      {
         _positions[i] = _initialPositions[i];
         _velocities[i] = _initialVelocities[i];
      }
   }
   
   glBindBuffer(GL_ARRAY_BUFFER, _pBO);
   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * _positions.size(), &_positions[0]);
   //   glVertexAttribPointer(_renderProg->getAttribLocation("pos"), 4, GL_FLOAT, GL_FALSE, 0, NULL);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*
 * Draw the particle system
 */
void ParticleSystem::draw()
{
   glBindVertexArray(_pVAO);
   glDrawArrays(GL_POINTS, 0, _positions.size());
   GL_ERR_CHECK();
   
}
#endif

