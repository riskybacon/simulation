//--------------------------------------------------------------------------------
// ocean.cpp
//
// Phillips spectrum initial conditions. Used by CAModelGLSL to set reasonable
// initial conditions.
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------

#include "ocean.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::dot;
using glm::normalize;
using glm::length;

/*
 * @return random value in range [0,1], uniformly distributed
 */
float uniformRandomVariable()
{
   return (float)rand()/RAND_MAX;
}

/*
 * @return a random complex number, normal distribution
 */
complex_type gaussianRandomVariable()
{
   float x1, x2, w;
   do
   {
      x1 = 2.f * uniformRandomVariable() - 1.f;
      x2 = 2.f * uniformRandomVariable() - 1.f;
      w = x1 * x1 + x2 * x2;
   } while ( w >= 1.f );
   w = sqrt((-2.f * log(w)) / w);
   return complex_type(x1 * w, x2 * w);
}

/*
 * Constructor
 *
 * @param N
 *    The number of cells in the lattice across one dimension (NxN lattice)
 * @param A
 *    Amplitude scaling factor. Choose 4
 * @param w
 *    Wind direction
 * @param length
 *    Size of the simulation in meters (length x length area)
 */
Ocean::Ocean(const int N, const float A, const glm::vec2 w, const float length)
: _g        (9.81)
, _N        (N)
, _Nplus1   (N+1)
, _A        (A)
, _w        (w)
, _length   (length)
{
   _pos.resize(_Nplus1 * _Nplus1);

   _hTilde       = (complex_type*) fftw_malloc(sizeof(complex_type) * _N * _N);
   _hTildePlan   = fftw_plan_dft_2d(N, N, reinterpret_cast<fftw_complex*>(_hTilde),
                                          reinterpret_cast<fftw_complex*>(_hTilde),
                                          FFTW_FORWARD, FFTW_ESTIMATE);
}

/*
 * Destructor
 */
Ocean::~Ocean()
{
   fftw_free(_hTilde);
   fftw_destroy_plan(_hTildePlan);
}


/*
 * Equation 33, 34 and 35 to model wave dispersion
 */
float Ocean::dispersion(int n_prime, int m_prime) const
{
   // Calculate w0, eqn 34
   float T = 200.0f; // Repeat after 200 iterations
	float w_0 = 2.0f * M_PI / T;
   
   // Create wavevector
	float kx = M_PI * (2 * n_prime - _N) / _length;
	float kz = M_PI * (2 * m_prime - _N) / _length;
   
   vec2 k(kx, kz);
   
   float k_length = glm::length(k);

   // Equation 33 - dispersion relation in deep water
   float eqn33 = sqrtf(_g * k_length * (1 + k_length * k_length * _length * _length));

   // Eqn 35
   return floor(eqn33 / w_0) * w_0;
}

/*
 * Phillips wave spectrum, equation 40 with modification specified in equation 41
 */
float Ocean::phillips(int n_prime, int m_prime) const
{
   // Wavevector
   float kx = M_PI * (2 * n_prime - _N) / _length;
   float kz = M_PI * (2 * m_prime - _N) / _length;
   vec2  k(kx, kz);
   
   // Magnitude of wavevector
   float k_length = glm::length(k);
   
   // Wind speed
	float w_length = glm::length(_w);

   // If wavevector is very small, no need to calculate, just return zero
   if (k_length < 0.000001) return 0.0;
   
   // Precaculate k^2 and k^4
	float k_length2 = k_length  * k_length;
	float k_length4 = k_length2 * k_length2;

   // Cosine factor - eliminates waves that travel perpendicular to the wind
   float k_dot_w = dot(normalize(k), normalize(_w));
	float k_dot_w2  = k_dot_w * k_dot_w;
   
   
	float L         = w_length * w_length / _g;
	float L2        = L * L;
	
   // Something a bit extra to keep waves from exploding. Not in the paper.
	float damping   = 0.001;
	float l2        = L2 * damping * damping;
   
   // Phillips spectrum as described in eqn  40 with modification described in 41 to
   // suppress very small waves that cause convergence problems
	return _A * (exp(-1.0f / (k_length2 * L2)) / k_length4) * k_dot_w2 * exp(-k_length2 * l2);
}

/*
 * Initial height amplitude at time 0. Equation 42
 *
 * @param n,m
 *    Position n,m on the lattice
 */
complex_type Ocean::hTilde_0(int n, int m) const
{
	complex_type r = gaussianRandomVariable();
	return r * sqrt(phillips(n, m) / 2.0f);
}


/*
 * Height amplitude function. Equation 43
 *
 * @param t
 *    Time in seconds
 * @param n,m
 *    Position n,m on the lattice
 */
complex_type Ocean::hTilde(float t, int n_prime, int m_prime) const
{
   // Calculate htilde0 and it's conjugate
	complex_type htilde0       =           hTilde_0( n_prime,  m_prime);
	complex_type htilde0mkconj = std::conj(hTilde_0(-n_prime, -m_prime));
   
   
	float omega_t = dispersion(n_prime, m_prime) * t;
   
	float cosOmegaT = cos(omega_t);
	float sinOmegaT = sin(omega_t);
   
	complex_type c0(cosOmegaT,  sinOmegaT);
	complex_type c1(cosOmegaT, -sinOmegaT);
   
	return htilde0 * c0 + htilde0mkconj * c1;
}

/*
 * Take the FFT of hTilde at time t, turn the result into positions
 */
void Ocean::evaluateWavesFFT(float t)
{
   // Fill _hTilde with height amplitude values
   int index = 0;
   for (int m_prime = 0; m_prime < _N; m_prime++)
   {
		for (int n_prime = 0; n_prime < _N; n_prime++, index++)
      {
         _hTilde[index] = hTilde(t, n_prime, m_prime);
      }
	}

   // Execute the FFT and get the height field
   fftw_execute(_hTildePlan);
   
	int sign;
	int index1;
	float signs[] = { 1.0f, -1.0f };

	for(int m_prime = 0; m_prime < _N; m_prime++)
   {
		for(int n_prime = 0; n_prime < _N; n_prime++)
      {
			index  = m_prime * _N + n_prime;		   // index into h_tilde.
			index1 = m_prime * _Nplus1 + n_prime;	// index into vertices

			sign = signs[(n_prime + m_prime) & 1];

         _hTilde[index] *= sign;

			// height
			_pos[index1].y = _hTilde[index].real();

			// for tiling
			if(n_prime == 0 && m_prime == 0)
         {
				_pos[index1 + _N + _Nplus1 * _N].y = _hTilde[index].real();
			}
			if(n_prime == 0)
         {
				_pos[index1 + _N].y = _hTilde[index].real();
			}
			if(m_prime == 0)
         {
				_pos[index1 + _Nplus1 * _N].y = _hTilde[index].real();
			}
		}
	}
}
