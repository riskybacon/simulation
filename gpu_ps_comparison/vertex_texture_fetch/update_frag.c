#version 150

//precision highp vec4;
//precision highp vec2;
//precision highp float;

uniform sampler2D inPos;
uniform sampler2D inVel;
uniform mat4 mv;


out vec4 newState[2];
in vec2 tc;

/**
 * Structures for passing around position and velocity state
 * and derivatives for use with RK4
 */
struct Derivative
{
   vec3 dx;
   vec3 dv;
};

struct State
{
   vec3 x;
   vec3 v;
};

const float m1 = 9.5e9;
const float m2 = 1e5;
const float invM1 = 1.0 / m1;
const float invM2 = 1.0 / m2;

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
// Output: acceleration vector (m/s)
// floating point ops: 6
//----------------------------------------------------------------------
vec3 acceleration(State state, float t)
{
   float g = gravity(m1, m2, length(state.x));
   vec3 force = normalize(state.x) * -1.0 * g;
   
   // Acceleration vector. F = ma => a = F/m
   return force * invM2;
}

//----------------------------------------------------------------------
// RK4 functions
// floating point ops: 6
//----------------------------------------------------------------------
Derivative evaluate(State initial, float t)
{
   Derivative OUT;
   OUT.dx = initial.v;
   OUT.dv = acceleration(initial, t);
   return OUT;
}

// floating point ops: 11
Derivative evaluate(State initial, float t, float dt, Derivative d)
{
   State state;
   state.x = initial.x + d.dx * dt;
   state.v = initial.v + d.dv * dt;
   
   Derivative OUT;
   OUT.dx = state.v;
   OUT.dv = acceleration(state, t+dt);
   
   return OUT;
}

// 6 + 11 + 11 + 11 + 10 + 4 = 53
State integrate(State state, float t, float dt)
{
   State OUT = state;
   
   Derivative a = evaluate(state, t);
   Derivative b = evaluate(state, t, dt*0.5f, a);
   Derivative c = evaluate(state, t, dt*0.5f, b);
   Derivative d = evaluate(state, t, dt, c);
   
   vec3 dxdt = 0.166666667f * (a.dx + 2.0f*(b.dx + c.dx) + d.dx);
   vec3 dvdt = 0.166666667f * (a.dv + 2.0f*(b.dv + c.dv) + d.dv);
   
   OUT.x = state.x + dxdt*dt;
   OUT.v = state.v + dvdt*dt;
   
   return OUT;
}

// 10 million particles: 530 million floating point ops per frame
void main(void)
{

   State before;
   before.x = texture(inPos, tc).rgb;
   before.v = texture(inVel, tc).rgb;
   
   State after = integrate(before, 0, 0.01);
   
   newState[0] = vec4(after.x, 1);
   newState[1] = vec4(after.v, 0);
}
