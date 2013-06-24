#version 150
//--------------------------------------------------------------------------------
// ca_update_frag.c
//
// Performs the update step for the cellular automata. This step is performed in
// a GLSL fragment shader
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------
uniform sampler2D pos;
uniform sampler2D massFlow0;
uniform sampler2D massFlow1;

in vec2 tc;

// Distance to step in texture space to get to the next texel
uniform float stepT;
uniform float stepS;
uniform float lambda;
uniform float timeStep;

// Output:
// calc[0] - position
// calc[1] - mass flow [f_0, f_1, f_2, f_3]
// calc[2] - mass flow [f_4, k, unused, unused]

out vec4 calc[3];

// Gravitational constant
const float g = 9.81;

/**
 * Dots row omega_i with the mass flow.
 */
float dotOmegaMass0(float k, const vec4 mf0, const vec4 mf1)
{
   float b = -4 * k;
   float c =  2 - b;
   return b * mf0.x + c * mf0.y + c * mf0.z + c * mf0.w + c * mf1.x;
}

float dotOmegaMass1(float k, const vec4 mf0, const vec4 mf1)
{
   float a = k - 1;
   return k * mf0.x + a * mf0.y + a * mf0.z + k * mf0.w + k * mf1.x;
}

float dotOmegaMass2(float k, const vec4 mf0, const vec4 mf1)
{
   float a = k - 1;
   return k * mf0.x + k * mf0.y + k * mf0.z + a * mf0.w + a * mf1.x;
}


void main(void)
{
   // Texture maps hold only 4 floating point components at
   // each site

   // Get the current position for this site in the lattice
   // from the pos texture map
   calc[0] = texture(pos, tc);

   // Get the first 4 components of the mass flow for this
   // site from the texture map:
   // [f_0, f_1, f_2, f_3]
   calc[1] = texture(massFlow0, tc);
   
   // Get the last component of the mass flow and the wave number
   // for this site from the texture map
   // [f_4, k, unused, unused]
   calc[2] = texture(massFlow1, tc);

   
   // Calculate new height - sum up f_0 through f_4
   calc[0].y = calc[1].x + calc[1].y + calc[1].z + calc[1].w + calc[2].x;

   calc[0].y = clamp(calc[0].y, -25, 25);

   // Velocity is lattice site spacing (meters) divided by length of time step (seconds)
   float v = lambda / timeStep;

   // Choose K for this site.
   // Remember that calc[2].y is the wave number
   float K = g / (v * v * calc[2].y);

   // The 4 texture map indices used for indexing
   // into the mass flow textures
   vec2 right = vec2(tc.s + stepS, tc.t);
   vec2 left  = vec2(tc.s - stepS, tc.t) ;
   vec2 up    = vec2(tc.s,         tc.t + stepT);
   vec2 down  = vec2(tc.s,         tc.t - stepT);

   // Testing code - to verify that the lookups are indeed picking values
   // one step up, left, right, or down, pull the height from the
   // neighboring pos() texel. The waves should stay static and move around the
   // mesh
   //
   // Uncomment the next line to perform this test
   //calc[0].y = texture(pos, right).y;

   // Mass flow temporary variables
   vec4 f_a;
   vec4 f_b;
   float dp;
   
   // New f_0
   dp = dotOmegaMass0(K, calc[1], calc[2]);
   //   calc[0].y = dp <= 0 ? 0 : calc[0].y;
   calc[1].x = calc[1].x + dp;
   
   // New f_1 - mass flow to the right.
   // Get the mass flow from the lattice neighbor to the left
   // and apply the update
   f_a = texture(massFlow0, left);
   f_b = texture(massFlow1, left);
   dp = dotOmegaMass1(K, f_a, f_b);
   //   calc[0].y = dp <= 0 ? 0 : calc[0].y + 0.5;
   calc[1].y = f_a.y + dp;
   
   // New f_2 - mass flow to the left
   // Get the mass flow from the lattice neighbor to the right
   // and apply the update
   f_a = texture(massFlow0, right);
   f_b = texture(massFlow1, right);
   dp = dotOmegaMass1(K, f_a, f_b);
   //   calc[0].y = dp <= 0 ? 0 : calc[0].y + 0.5;
   calc[1].z = f_a.z + dp;
   
   // New f_3 - mass flow upwards
   // Get the mass flow from the lattice neighbor to the bottom
   // and apply the update
   f_a = texture(massFlow0, up);
   f_b = texture(massFlow1, up);
   dp = dotOmegaMass2(K, f_a, f_b);
   //   calc[0].y = dp <= 0 ? 0 : calc[0].y + 0.5;
   calc[1].w = f_a.w + dp;
   
   // New f_4 - mass flow downwards
   // Get the mass flow from the lattice neighbor to the top
   // and apply the update
   f_a = texture(massFlow0, down);
   f_b = texture(massFlow1, down);
   dp = dotOmegaMass2(K, f_a, f_b);
   //   calc[0].y = dp <= 0 ? 0 : calc[0].y + 0.5;
   calc[2].x = f_b.x + dp;
}
