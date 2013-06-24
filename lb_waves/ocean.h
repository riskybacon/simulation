//--------------------------------------------------------------------------------
// ocean.h
//
// Phillips spectrum initial conditions. Used by CAModelGLSL to set reasonable
// initial conditions.
//
// See the paper "Simulating Ocean Water" by Jerry Tessendorf for details
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------
#ifndef _ocean_h
#define _ocean_h

#include <glm/glm.hpp>
#include <complex>
#include <fftw3.h>
#include <vector>

typedef std::complex<double> complex_type;

/**
 * Initial ocean-like conditions
 * @param N
 *    The number of cells in the lattice across one dimension (NxN lattice)
 * @param A
 *    Amplitude scaling factor. Choose 4
 * @param w
 *    Wind direction
 * @param length
 *    Size of the simulation in meters (length x length area)
 */
class Ocean
{
public:
   typedef std::complex<double> complex_type;

   /**
    * Constructor
    */
	Ocean(const int N, const float A, const glm::vec2 w, const float length);
   
   /**
    * Destructor
    */
	~Ocean();
   
   /*
    * Equation 33, 34 and 35 to model wave dispersion in deep water
    */
	float dispersion(int n_prime, int m_prime) const;

	/**
    * Phillips wave spectrum, equation 40 with modification specified in equation 41
    */
   float phillips(int n_prime, int m_prime) const;
   
   /**
    * Initial height amplitude at time 0. Equation 42.
    *
    * @param n,m
    *    Position n,m on the lattice
    */
	complex_type hTilde_0(int n_prime, int m_prime) const;
   
   /**
    * Height amplitude function. Equation 43
    *
    * @param t
    *    Time in seconds
    * @param n,m
    *    Position n,m on the lattice
    *
    */
   complex_type hTilde(float t, int n_prime, int m_prime) const;
   
   /**
    * Take the FFT of hTilde at time t, turn the result into positions
    */
	void evaluateWavesFFT(float t);

   /**
    * @return the positions of the vertices in the lattice
    */
   const std::vector<glm::vec4>& getVertices() const
   {
      return _pos;
   }

private:
   
	float                   _g;            //< Gravitational constant
	int                     _N;            //< Dimension of the lattice. Try and make it a power of 2
   int                     _Nplus1;       //< N + 1
	float                   _A;            //< Phillips spectrum "scaling constant" parameter. Changes the heights of waves
   glm::vec2               _w;            //< Wind directionameter

	float                   _length;			//< Size of simulation in meters (_length x _length area)

   // For FFT
   complex_type*           _hTilde;
   fftw_plan               _hTildePlan;

   std::vector<glm::vec4>  _pos;          //< Lattice positions
};

#endif

