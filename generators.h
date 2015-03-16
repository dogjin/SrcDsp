/*-----------------------------------------------------------------------------
@file

Definition of all DSP routines related waveform generators
The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#ifndef _GENERATOR_H
#define _GENERATOR_H

#include <cassert>
#include <vector>
#include <complex>
#include <cmath>
#include "constants.h"

// Uncomment the following line to include the C++ specific syntax
//#define CPLUSPLUS11

namespace dsptl
{


	/*-----------------------------------------------------------------------------
	Sinewave generator. Generates sinewaves

	@tparam OutType Type of the output signal

	@param frequency Sinewave frequency in normalized units between 0 and 1
	@param amplitude Peak amplitude of the sinewave

	The computation are done internally as double and converted to the desired type
	with the desired amplitude\n
	The class has a specialization for the complex output type

	------------------------------------------------------------------------------*/
	template<class OutType>
	class GenSine
	{
	public:
		/// Constructor. 
		GenSine(double frequency, OutType amplitude) : ampl(amplitude)
		{
			freq = frequency * pi;
			phase = 0;
		}
		// Generate a number of samples equal to the size of the buffer given
		void step(std::vector<OutType>& out)
		{
			for (unsigned i = 0; i < out.size(); ++i)
			{
				phase += freq;
				if (phase >= 2 * pi) phase -= 2 * pi;
				out[i] = static_cast<OutType>(cos(phase) * ampl);
			}

		}
		/// reset the internal phase of the generator
		void reset()
		{
			phase = 0;
		}

	private:
		OutType ampl; // output level
		double freq; //rad_per_sample
		double phase;  //radians
	};

	template<class OutType>
	class GenSine<std::complex<OutType> >
	{
	public:
		/// Constructor. 
		GenSine(double frequency, OutType amplitude) :  ampl(amplitude)
		{
			phase = 0;
			freq = frequency * pi;
		}
		// Generate a number of samples equal to the size of the buffer given
		void step(std::vector<std::complex<OutType> >& out)
		{
			for (unsigned i = 0; i < out.size(); ++i)
			{
				phase += freq;
				if (phase >= 2 * pi) phase -= 2 * pi;
				out[i] = std::complex<OutType>(static_cast<OutType>(cos(phase) * ampl), static_cast<OutType>(sin(phase) * ampl));
			}

		}
		/// reset the internal phase of the generator
		void reset()
		{
			phase = 0;
		}

	private:
		OutType ampl; // output level
		double freq; //rad_per_sample
		double phase;  //radians
	};



} // End of namespace

#endif
